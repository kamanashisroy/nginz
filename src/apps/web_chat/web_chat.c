
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
#include "apps/web_chat/web_chat.h"

C_CAPSULE_START

#define HTTP_WEBCHAT_PLUG "http/webchat"
//#define WEBCHAT_HIDDEN_JOIN_PREFIX "http/_webchat_hiddenjoin"

int webchat_transfer_parallel(struct http_connection*http, int destpid, aroop_txt_t*sid, aroop_txt_t*rest) {
	aroop_txt_t cmd = {};
	aroop_txt_embeded_stackbuffer(&cmd, 128);
	aroop_txt_concat_string_len(&cmd, HTTP_WEBCHAT_PLUG, sizeof(HTTP_WEBCHAT_PLUG)-1);
	aroop_txt_concat_char(&cmd, ' ');
	aroop_txt_concat(&cmd, sid);
	aroop_txt_concat_char(&cmd, '\n');
	aroop_txt_concat(&cmd, rest);
	int ret = default_transfer_parallel(&http->strm, destpid, NGINZ_HTTP_PORT, &cmd);
	http->state |= CHAT_SOFT_QUIT; // quit the user from this process
	//syslog(LOG_NOTICE, "transferred to %d %s\n", destpid, aroop_txt_to_string(&cmd));
	aroop_txt_destroy(&cmd);
	return ret;
}

static struct web_session_hooks*web_hooks = NULL;

static struct web_session_connection* http_webchat_create_helper(struct http_connection*http, aroop_txt_t*plugin_space, aroop_txt_t*request) {
	aroop_assert(http->strm.bubble_down == NULL);
	aroop_assert(web_hooks);
	struct web_session_connection*webchat = web_hooks->alloc(http->strm.fd, NULL);/* This fd may be INVALID */
	if(webchat == NULL) {
		syslog(LOG_ERR, "Could not create a chat connection\n");
		return NULL;
	}
	streamio_chain(&http->strm, &webchat->strm); // it increases reference count for [chat]
#if 0
	http->strm.transfer_parallel = webchat_transfer_parallel;
#endif
	struct chat_connection*chat = (struct chat_connection*)webchat->strm.bubble_down;
	chat->request = request; // the real request
	composite_plugin_bridge_call(chat_plugin_manager_get(), plugin_space, CHAT_SIGNATURE, chat);
	chat->request = NULL; // cleanup
	return webchat;
}

#if 0
static int http_webchat_hiddenjoin_plug(int signature, void*given) {
	aroop_assert(signature == HTTP_SIGNATURE);
	struct http_connection*http = (struct http_connection*)given;
	if(!IS_VALID_HTTP(http)) // sanity check
		return 0;
	aroop_txt_t hidden_join_cmd = {};
	aroop_txt_embeded_rebuild_copy_shallow(&hidden_join_cmd, &http->content); // needs cleanup
	aroop_txt_shift(&hidden_join_cmd, sizeof(WEBCHAT_HIDDEN_JOIN_PREFIX));
	aroop_txt_shift(&hidden_join_cmd, 1); // skip the space
	//aroop_txt_zero_terminate(&hidden_join_cmd);syslog(LOG_NOTICE, "Doing hidden join for[%s]\n", aroop_txt_to_string(&hidden_join_cmd));
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/_hiddenjoin");
	http_webchat_create_helper(http, &plugin_space, &hidden_join_cmd);
	aroop_txt_destroy(&hidden_join_cmd);
	aroop_txt_destroy(&plugin_space);
	return 0;
}
#endif

static int http_webchat_get_sessionid_trim_webchat_prefix(aroop_txt_t*sid) {
	int reallen = aroop_txt_length(sid);
	aroop_txt_set_length(sid, sizeof(HTTP_WEBCHAT_PLUG)-1);
	if(aroop_txt_equals_static(sid, HTTP_WEBCHAT_PLUG)) {
		aroop_txt_set_length(sid, reallen);
		int skip = sizeof(HTTP_WEBCHAT_PLUG);
		aroop_txt_shift(sid, skip);
	} else {
		aroop_txt_set_length(sid, reallen);
	}
	return 0;
}

static int http_webchat_get_sessionid(struct http_connection*http, aroop_txt_t*sid, aroop_txt_t*rest, int*pid) { // needs sid cleanup
	aroop_assert(http && sid); // sanity check
	if(aroop_txt_is_empty(&http->content)) { // we need the session id
		aroop_txt_destroy(sid); // cleanup
		aroop_txt_destroy(rest); // cleanup
		return -1;
	}
	aroop_txt_embeded_copy_deep(sid, &http->content); // needs cleanup
	aroop_txt_zero_terminate(sid);
	aroop_assert(aroop_txt_is_zero_terminated(sid));
	//syslog(LOG_NOTICE, "parsing = %s", aroop_txt_to_string(sid));
	// if it is bubbled command then remove the prefix
	http_webchat_get_sessionid_trim_webchat_prefix(sid);
	syslog(LOG_NOTICE, " -- parsing \n = %s", aroop_txt_to_string(sid));
	aroop_txt_embeded_rebuild_copy_shallow(rest, sid); // needs cleanup
	char*content = aroop_txt_to_string(sid);
	char*newline = strchr(content, '\n');
	if(!newline) {
		aroop_txt_destroy(sid); // cleanup
		aroop_txt_destroy(rest); // cleanup
		return -1;
	}
	int sidlen = newline - content;
	aroop_txt_set_length(sid, sidlen);
	aroop_txt_zero_terminate(sid);
	aroop_txt_shift(rest, sidlen+1);
	aroop_txt_t intval = {};
	aroop_txt_embeded_stackbuffer_from_txt(&intval, sid);
	char*dash = strchr(content, '-');
	if(!dash) {
		aroop_txt_destroy(sid); // cleanup
		aroop_txt_destroy(rest); // cleanup
		return -1;
	}
	int intlen = dash-content;
	aroop_txt_set_length(&intval, intlen);
	aroop_txt_zero_terminate(&intval);
	*pid = aroop_txt_to_int(&intval);
	//syslog(LOG_NOTICE, "intval = %d %s\n", *pid, aroop_txt_to_string(sid));
	aroop_txt_destroy(&intval);
	return 0;
}

static int http_webchat_plug(int signature, void*given) {
	/* ************************************ */
	/*           do some checkup            */
	/* ************************************ */
	aroop_assert(signature == HTTP_SIGNATURE);
	struct http_connection*http = (struct http_connection*)given;
	if(!IS_VALID_HTTP(http)) // sanity check
		return 0;
	/* ************************************ */
	/*     Parse the session id and pid     */
	/* ************************************ */
	aroop_txt_t sid = {};
	aroop_txt_t rest = {};
	int pid = -1;
	http_webchat_get_sessionid(http, &sid, &rest, &pid); // needs cleanup
	int ret = 0;
	struct web_session_connection*webchat = NULL;
	syslog(LOG_NOTICE, "rest %s\n", aroop_txt_to_string(&rest));
	http->is_processed = 0;
	do {
		if(pid != -1 && pid != getpid()) {
			// it is not ours 
			ret = webchat_transfer_parallel(http, pid, &sid, &rest);
			break;
		}

		webchat = (struct web_session_connection*)http->strm.bubble_down;
		if(!webchat && !aroop_txt_is_empty(&sid)) {
			/* ************************************ */
			/*      Find the session by id          */
			/* ************************************ */
			aroop_assert(web_hooks);
			syslog(LOG_NOTICE, "searching %d %s\n", aroop_txt_length(&sid), aroop_txt_to_string(&sid));
			webchat = web_hooks->search(&sid);
			if(webchat) {
				syslog(LOG_NOTICE, "found %s\n", aroop_txt_to_string(&rest));
				streamio_chain(&http->strm, &webchat->strm); // it increases reference count for [chat]
			}
		}
		if(!webchat) {
			syslog(LOG_NOTICE, "You are new .. \n");
			/* ************************************ */
			/*      Create new session+chat         */
			/* ************************************ */
			aroop_txt_t welcome_command = {};
			aroop_txt_embeded_set_static_string(&welcome_command, "chat/_welcome");
			webchat = http_webchat_create_helper(http, &welcome_command, NULL);
			break;
		}
		if(!webchat) {
			ret = -1;
			break;
		}
		syslog(LOG_NOTICE, "You came back .. \n");
		if(http->is_processed) {
			syslog(LOG_NOTICE, "Nothing to do .. \n");
			break;
		}
		/* ************************************ */
		/*      Process chat data               */
		/* ************************************ */
		//syslog(LOG_NOTICE, "content:%s\n", aroop_txt_to_string(&http->content));
		//chat->strm.on_recv(&chat->strm, http->request_data);
		syslog(LOG_NOTICE, "calling chat on_recv .. %s \n", aroop_txt_to_string(&rest));
		struct chat_connection*chat = (struct chat_connection*)webchat->strm.bubble_down;
		chat->strm.on_recv(&chat->strm, &rest);
		if(!http->is_processed) {
			// send http OK to let them know we are working ..
			http->strm.send(&http->strm, NULL, 0);
		}
	}while(0);
	if(webchat) {
		streamio_unchain(&http->strm, &webchat->strm); // unchain and decrease reference count of webchat
	}
	aroop_txt_destroy(&sid); // cleanup
	aroop_txt_destroy(&rest); // cleanup
	return ret;
}

static int http_webchat_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "webchat", "http", plugin_space, __FILE__, "It decorates the chat io for HTTP tunneling.\n");
}

static int web_session_api_plug(int signature, void*given) {
	aroop_assert(signature = WEB_CHAT_SIGNATURE);
	web_hooks = (struct web_session_hooks*)given;
	aroop_assert(web_hooks != NULL);
	return 0;
}

static int web_session_api_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "web_chat_chatapi", "chat hooking", plugin_space, __FILE__, "It reads the chat-api hooks.\n");
}


int web_chat_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, HTTP_WEBCHAT_PLUG);
	composite_plug_bridge(http_plugin_manager_get(), &plugin_space, http_webchat_plug, http_webchat_plug_desc);
#if 0
	aroop_txt_embeded_set_static_string(&plugin_space, WEBCHAT_HIDDEN_JOIN_PREFIX);
	composite_plug_bridge(http_plugin_manager_get(), &plugin_space, http_webchat_hiddenjoin_plug, http_webchat_plug_desc);
#endif
	aroop_txt_embeded_set_static_string(&plugin_space, "web_session/api/hookup");
	pm_plug_bridge(&plugin_space, web_session_api_plug, web_session_api_plug_desc);
	page_chat_module_init();
	web_session_factory_module_init();
	return 0;
}

int web_chat_module_deinit() {
	web_session_factory_module_deinit();
	page_chat_module_deinit();
	pm_unplug_bridge(0, web_session_api_plug);
	composite_unplug_bridge(http_plugin_manager_get(), 0, http_webchat_plug);
#if 0
	composite_unplug_bridge(http_plugin_manager_get(), 0, http_webchat_hiddenjoin_plug);
#endif
	return 0;
}


C_CAPSULE_END
