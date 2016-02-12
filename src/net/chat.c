
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"

C_CAPSULE_START

static int chat_destroy(struct chat_connection*chat) {
	// cleanup socket
	event_loop_unregister_fd(chat->fd);
	if(chat->fd != -1 && chat->state != CHAT_SOFT_QUIT)close(chat->fd);
	chat->fd = -1;
	// free data
	OPPUNREF(chat);
}

static int chat_command(struct chat_connection*chat, aroop_txt_t*given_request) {
	int ret = 0;
	// make a copy
	aroop_txt_t request = {};
	aroop_txt_embeded_txt_copy_shallow(&request, given_request);
	aroop_txt_shift(&request, 1); // skip '/' before command

	// get the command token
	aroop_txt_t ctoken = {};
	shotodol_scanner_next_token(&request, &ctoken);
	do {
		if(aroop_txt_is_empty_magical(&ctoken)) {
			// we cannot handle the data
			ret = -1;
			break;
		}
		aroop_txt_shift(&request, 1); // skip SPACE after command
		chat->request = &request;
		aroop_txt_t plugin_space = {};
		aroop_txt_embeded_stackbuffer(&plugin_space, 64);
		aroop_txt_concat_string(&plugin_space, "chat/");
		aroop_txt_concat(&plugin_space, &ctoken);
		ret = composite_plugin_bridge_call(chat_plugin_manager_get(), &plugin_space, CHAT_SIGNATURE, chat);
		aroop_txt_destroy(&plugin_space);
		chat->request = NULL; // cleanup
	} while(0);
	aroop_txt_destroy(&ctoken);
	aroop_txt_destroy(&request);
	return ret;
}

static aroop_txt_t cannot_process = {};
static aroop_txt_t recv_buffer = {};
static int on_client_data(int status, const void*cb_data) {
	struct chat_connection*chat = (struct chat_connection*)cb_data;
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	int count = recv(chat->fd, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	if(count == 0) {
		printf("Client disconnected\n");
		chat_destroy(chat);
		return -1;
	}
	aroop_txt_set_length(&recv_buffer, count);
	if(chat->on_answer != NULL) {
		chat->on_answer(chat, &recv_buffer);
		return 0;
	}
	do {
		// check if it is a command
		if(aroop_txt_char_at(&recv_buffer, 0) == '/') {
			if(!chat_command(chat, &recv_buffer)) {
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
		printf("Client quited\n");
		chat_destroy(chat);
		return -1;
	}
	return 0;
}

static struct opp_factory chat_factory;
static int chat_on_guest_hookup(int fd, aroop_txt_t*cmd) {
	struct chat_connection*chat = OPP_ALLOC1(&chat_factory);
	chat->fd = fd;
	aroop_txt_t x = {};
	binary_unpack_string(cmd, 1, &x);
	chat->request = &x;
	aroop_txt_t plugin_space = {};
	int reqlen = aroop_txt_length(chat->request);
	aroop_txt_t request_sandbox = {};
	aroop_txt_embeded_stackbuffer(&request_sandbox, reqlen);
	aroop_txt_concat(&request_sandbox, chat->request);
	shotodol_scanner_next_token(&request_sandbox, &plugin_space);
	if(aroop_txt_is_empty(&plugin_space)) {
		aroop_txt_zero_terminate(&request_sandbox);
		printf("Possible BUG , cannot handle request %s", aroop_txt_to_string(&request_sandbox));
		return -1;
	}

	composite_plugin_bridge_call(chat_plugin_manager_get(), &plugin_space, CHAT_SIGNATURE, chat);
	printf("adding guest client to event loop\n");
	event_loop_register_fd(fd, on_client_data, chat, NGINZ_POLL_ALL_FLAGS);
	return 0;
}

struct protostack chat_protostack = {
	.on_guest_hookup = chat_on_guest_hookup,
};

OPP_CB(chat_connection) {
	struct chat_connection*chat = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			aroop_memclean_raw2(&chat->name);
			chat->on_answer = NULL;
			chat->on_broadcast = NULL;
			chat->state = CHAT_CONNECTED;
			chat->request = NULL;
		break;
		case OPPN_ACTION_FINALIZE:
			aroop_txt_destroy(&chat->name);
		break;
	}
	return 0;
}



int chat_module_init() {
	chat_plugin_manager_module_init();
	OPP_PFACTORY_CREATE(&chat_factory, 64, sizeof(struct chat_connection), OPP_CB_FUNC(chat_connection));
	aroop_txt_embeded_set_static_string(&cannot_process, "Cannot process the request\n");
	aroop_txt_embeded_buffer(&recv_buffer, 255);
	protostack_set(NGINZ_DEFAULT_PORT, &chat_protostack);
}

int chat_module_deinit() {
	aroop_txt_destroy(&recv_buffer);
	// TODO  unplug all the clients
	struct opp_iterator iterator;
	opp_iterator_create(&iterator, &chat_factory, OPPN_ALL, 0, 0);
	do {
		struct chat_connection*chat = opp_iterator_next(&iterator);
		if(chat == NULL)
			break;
		event_loop_unregister_fd(chat->fd);
		close(chat->fd);
		chat->fd = -1;
		//OPPUNREF(plugin);
	} while(1);
	opp_iterator_destroy(&iterator);
	OPP_PFACTORY_DESTROY(&chat_factory);
	chat_plugin_manager_module_deinit();
}


C_CAPSULE_END
