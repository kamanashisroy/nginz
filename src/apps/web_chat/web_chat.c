
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "log.h"
#include "net/chat.h"
#include "net/http.h"
#include "net/http_tunnel.h"

C_CAPSULE_START

static int web_chat_send_content(aroop_txt_t*content, int flag) {
	return http_tunnel_send_content(http, content);
}

static int http_webchat_plug(int signature, void*given) {
	aroop_assert(signature == HTTP_SIGNATURE);
	struct http_connection*http = (struct http_connection*)given;
	if(http == NULL || http->fd == -1) // sanity check
		return 0;
	if(http->state != HTTP_RESERVED4) {
		event_loop_
	}
	struct chat_connection*chat = (struct chat_connection*)http->app_data;
	if(!chat) {
		chat = chathooks->on_create(fd);
		if(chat == NULL) {
			syslog(LOG_ERR, "Could not create a chat connection\n");
			return 0;
		}
		chat->send_content = web_chat_send_content;
		http->app_data = chat;
	}
	chat_hooks->handle_chat_request(chat, http->request_data);
	return 0;
}

static int http_webchat_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "webchat", "http", plugin_space, __FILE__, "It shows available commands.\n");
}


int web_chat_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "http/webchat");
	composite_plug_bridge(http_plugin_manager_get(), &plugin_space, http_webchat_plug, http_webchat_plug_desc);
	return 0;
}

int web_chat_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, http_webchat_plug);
	return 0;
}


C_CAPSULE_END
