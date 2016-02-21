
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "log.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/http.h"
#include "net/http/http_plugin_manager.h"
//#include "net/http/http_tunnel.h"

C_CAPSULE_START

/*static int web_chat_send_content(aroop_txt_t*content, int flag) {
	return http_tunnel_send_content(http, content);
}*/

#define WEBCHAT_HIDDEN_JOIN_PREFIX "http/_webchat_hiddenjoin "

int webchat_transfer_parallel(struct streamio*strm, int destpid, int proto_port, aroop_txt_t*cmd) {
	aroop_txt_t wrapper_cmd = {};
	int cmdlen = aroop_txt_length(cmd) + sizeof(WEBCHAT_HIDDEN_JOIN_PREFIX) + 1;
	aroop_txt_embeded_stackbuffer(&wrapper_cmd, cmdlen);
	aroop_txt_concat_string(&wrapper_cmd, WEBCHAT_HIDDEN_JOIN_PREFIX);
	aroop_txt_concat(&wrapper_cmd, cmd);
	aroop_txt_zero_terminate(&wrapper_cmd);
	int ret = default_transfer_parallel(strm, destpid, proto_port, &wrapper_cmd);
	struct http_connection*http = (struct http_connection*)strm;
	http->state |= CHAT_SOFT_QUIT; // quit the user from this process
	return ret;
}

static struct chat_hooks*hooks;

static int http_webchat_create_helper(struct http_connection*http, aroop_txt_t*plugin_space, aroop_txt_t*request) {
	aroop_assert(http->strm.bubble_down == NULL);
	struct chat_connection*chat = hooks->on_create(http->strm.fd);/* This fd may be INVALID */
	if(chat == NULL) {
		syslog(LOG_ERR, "Could not create a chat connection\n");
		return -1;
	}
	streamio_chain(&http->strm, &chat->strm); // it increases reference count for [chat]
	http->strm.transfer_parallel = webchat_transfer_parallel;
	chat->request = request; // the real request
	composite_plugin_bridge_call(chat_plugin_manager_get(), plugin_space, CHAT_SIGNATURE, chat);
	chat->request = NULL; // cleanup
	OPPUNREF(chat); // cleanup
	return 0;
}

static int http_webchat_hiddenjoin_plug(int signature, void*given) {
	aroop_assert(signature == HTTP_SIGNATURE);
	struct http_connection*http = (struct http_connection*)given;
	if(!IS_VALID_CHAT(http)) // sanity check
		return 0;
	aroop_txt_t hidden_join_cmd = {};
	aroop_txt_embeded_rebuild_copy_shallow(&hidden_join_cmd, &http->content); // needs cleanup
	aroop_txt_shift(&hidden_join_cmd, sizeof(WEBCHAT_HIDDEN_JOIN_PREFIX));
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/_hiddenjoin");
	http_webchat_create_helper(http, &plugin_space, &hidden_join_cmd);
	aroop_txt_destroy(&hidden_join_cmd);
	aroop_txt_destroy(&plugin_space);
	return 0;
}

static int http_webchat_plug(int signature, void*given) {
	aroop_assert(signature == HTTP_SIGNATURE);
	struct http_connection*http = (struct http_connection*)given;
	if(!IS_VALID_CHAT(http)) // sanity check
		return 0;
	struct chat_connection*chat = (struct chat_connection*)http->strm.bubble_down;
	if(!chat) {
		aroop_txt_t welcome_command = {};
        	aroop_txt_embeded_set_static_string(&welcome_command, "chat/_welcome");
		http_webchat_create_helper(http, &welcome_command, NULL);
		return 0;
	}
	syslog(LOG_NOTICE, "content:%s\n", aroop_txt_to_string(&http->content));
	//chat->strm.on_recv(&chat->strm, http->request_data);
	chat->strm.on_recv(&chat->strm, &http->content);
	return 0;
}

static int http_webchat_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "webchat", "http", plugin_space, __FILE__, "It decorates the chat io for HTTP tunneling.\n");
}

static int web_chat_chatapi_hookup(int signature, void*given) {
	hooks = (struct chat_hooks*)given;
	aroop_assert(hooks != NULL);
	return 0;
}

static int web_chat_chatapi_hookup_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "web_chat_chatapi", "chat hooking", plugin_space, __FILE__, "It reads the chat-api hooks.\n");
}


int web_chat_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "http/webchat");
	composite_plug_bridge(http_plugin_manager_get(), &plugin_space, http_webchat_plug, http_webchat_plug_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "http/_webchat_hiddenjoin");
	composite_plug_bridge(http_plugin_manager_get(), &plugin_space, http_webchat_hiddenjoin_plug, http_webchat_plug_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "chatproto/hookup");
	pm_plug_bridge(&plugin_space, web_chat_chatapi_hookup, web_chat_chatapi_hookup_desc);
	return 0;
}

int web_chat_module_deinit() {
	pm_unplug_bridge(0, web_chat_chatapi_hookup);
	composite_unplug_bridge(http_plugin_manager_get(), 0, http_webchat_plug);
	composite_unplug_bridge(http_plugin_manager_get(), 0, http_webchat_hiddenjoin_plug);
	return 0;
}


C_CAPSULE_END
