
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/chat.h"
#include "net/chat/chat_accept.h"

C_CAPSULE_START

static int chat_module_is_quiting = 0;
static struct chat_hooks*hooks = NULL;
static int chat_accept_on_softquit(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	chat_module_is_quiting = 1;
	return 0;
}

static int chat_accept_on_softquit_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "softquitall", "shake", plugin_space, __FILE__, "It does not allow new connection.\n");
}

static aroop_txt_t cannot_process = {};
static aroop_txt_t recv_buffer = {};
static int on_client_data(int fd, int status, const void*cb_data) {
	if(chat_module_is_quiting)
		return 0;
	struct chat_connection*chat = (struct chat_connection*)cb_data;
	aroop_assert(fd == chat->fd);
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	int count = recv(chat->fd, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	if(count == 0) {
		syslog(LOG_INFO, "Client disconnected\n");
		hooks->on_destroy(&chat);
		return -1;
	}
	if(count >= NGINZ_MAX_CHAT_MSG_SIZE) {
		syslog(LOG_INFO, "Disconnecting client for too big data input\n");
		hooks->on_destroy(&chat);
		return -1;
	}
	aroop_txt_set_length(&recv_buffer, count);
	int last_index = count-1;
	if(aroop_txt_char_at(&recv_buffer, last_index) != '\n') {
		syslog(LOG_INFO, "Disconnecting client for unrecognized data\n");
		hooks->on_destroy(&chat);
		return -1;
	}
	if(chat->on_answer != NULL) {
		chat->on_answer(chat, &recv_buffer);
		return 0;
	}
	do {
		// check if it is a command
		if(aroop_txt_char_at(&recv_buffer, 0) == '/') {
			if(!hooks->on_command(chat, &recv_buffer)) {
				break;
			}
		} else if(chat->on_broadcast != NULL) {
			chat->on_broadcast(chat, &recv_buffer);
			break;
		}
		if(chat->state == CHAT_QUIT || chat->state == CHAT_SOFT_QUIT) {
			// time to quit
			break;
		}
		// we cannot handle data
		send(chat->fd, aroop_txt_to_string(&cannot_process), aroop_txt_length(&cannot_process), 0);
	} while(0);
	if(chat->state == CHAT_QUIT || chat->state == CHAT_SOFT_QUIT) {
		syslog(LOG_INFO, "Client quited\n");
		hooks->on_destroy(&chat);
		return -1;
	}
	return 0;
}

static int on_tcp_connection(int fd) {
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, 0);
	binary_pack_int(&bin, NGINZ_CHAT_PORT);
	aroop_txt_t welcome_command = {};
	aroop_txt_embeded_set_static_string(&welcome_command, "chat/_welcome"); 
	binary_pack_string(&bin, &welcome_command);
	pp_bubble_down_send_socket(fd, &bin);
	return 0;
}

static int on_connection_bubble(int fd, aroop_txt_t*cmd) {
	aroop_assert(hooks != NULL);
	if(chat_module_is_quiting)
		return 0;
	int ret = 0;
	// create new connection
	struct chat_connection*chat = hooks->on_create(fd);
	aroop_txt_t x = {};
	binary_unpack_string(cmd, 2, &x); // needs cleanup
	chat->request = &x; // set the request/command

	do {

		// get the target command
		aroop_txt_t plugin_space = {};
		int reqlen = aroop_txt_length(chat->request);
		aroop_txt_t request_sandbox = {};
		aroop_txt_embeded_stackbuffer(&request_sandbox, reqlen);
		aroop_txt_concat(&request_sandbox, chat->request);
		shotodol_scanner_next_token(&request_sandbox, &plugin_space);
		if(aroop_txt_is_empty(&plugin_space)) {
			aroop_txt_zero_terminate(&request_sandbox);
			syslog(LOG_ERR, "Possible BUG , cannot handle request %s", aroop_txt_to_string(&request_sandbox));
			hooks->on_destroy(&chat); // cleanup
			ret = -1;
			break;
		}

		// register it in the event loop
		event_loop_register_fd(fd, on_client_data, chat, NGINZ_POLL_ALL_FLAGS);

		// execute the command
		composite_plugin_bridge_call(chat_plugin_manager_get(), &plugin_space, CHAT_SIGNATURE, chat);
	} while(0);
	chat->request = NULL; // cleanup
	aroop_txt_destroy(&x); // cleanup
	return ret;
}

static int chat_accept_hookup(int signature, void*given) {
	struct chat_hooks*ghooks = (struct chat_hooks*)given;
	hooks = ghooks;
	return 0;
}

static int chat_accept_hookup_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "chat_accept", "chat hooking", plugin_space, __FILE__, "It registers connection creation and destruction hooks.\n");
}

static struct protostack chat_protostack = {
	.on_tcp_connection = on_tcp_connection,
	.on_connection_bubble = on_connection_bubble,
};

int chat_accept_module_init() {
	aroop_txt_embeded_set_static_string(&cannot_process, "Cannot process the request\n");
	aroop_txt_embeded_buffer(&recv_buffer, NGINZ_MAX_CHAT_MSG_SIZE);
	protostack_set(NGINZ_CHAT_PORT, &chat_protostack);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chatproto/hookup");
	pm_plug_bridge(&plugin_space, chat_accept_hookup, chat_accept_hookup_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, chat_accept_on_softquit, chat_accept_on_softquit_desc);
}

int chat_accept_module_deinit() {
	protostack_set(NGINZ_HTTP_PORT, NULL);
	pm_unplug_bridge(0, chat_accept_hookup);
	pm_unplug_callback(0, chat_accept_on_softquit);
	aroop_txt_destroy(&recv_buffer);
}

C_CAPSULE_END
