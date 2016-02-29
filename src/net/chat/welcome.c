
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "log.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/user.h"
#include "net/chat/welcome.h"

C_CAPSULE_START

#if 0
struct chat_hooks*hooks = NULL;
#endif
aroop_txt_t greet_on_login = {};
static int on_login_complete(int token, aroop_txt_t*name, int success) {
	if(aroop_txt_is_empty_magical(name)) {
		syslog(LOG_ERR, "Error, we do not know user name\n");
		return 0;
	}
	struct chat_connection*chat = chat_api_get()->get(token); // needs cleanup
	if(!chat) {
		syslog(LOG_ERR, "Error, could not find the logged in user\n");
		return 0;
	}
	if(success) {
		// save name
		aroop_txt_embeded_copy_deep(&chat->name, name);
		// say welcome
		aroop_txt_set_length(&greet_on_login, 0);
		aroop_txt_concat_string(&greet_on_login, "Welcome ");
		aroop_txt_concat(&greet_on_login, name);
		aroop_txt_concat_char(&greet_on_login, '!');
		aroop_txt_concat_char(&greet_on_login, '\n');
		chat->on_response_callback = NULL;
		chat->state |= CHAT_LOGGED_IN;
	} else {
		aroop_txt_set_length(&greet_on_login, 0);
		aroop_txt_concat_string(&greet_on_login, "Sorry, name taken.\n");
	}
	if(!aroop_txt_is_empty(&greet_on_login))
		chat->strm.send(&chat->strm, &greet_on_login, 0);
	OPPUNREF(chat);
	return 0;
}

static int on_login_data(struct chat_connection*chat, aroop_txt_t*answer) {
	if(aroop_txt_is_empty_magical(answer))
		return 0;
	aroop_assert(chat->state == CHAT_CONNECTED);
	// remove the trailing space and new line
	aroop_txt_t input = {};
	aroop_txt_embeded_txt_copy_shallow(&input, answer);
	aroop_txt_t name = {};
	shotodol_scanner_next_token(&input, &name); // needs cleanup
	do {
		// check validity
		if(aroop_txt_is_empty_magical(&name)) {
			aroop_txt_set_length(&greet_on_login, 0);
			aroop_txt_concat_string(&greet_on_login, "Invalid name:(, try again\n");
			break;
		}
		if(aroop_txt_length(&name) >= NGINZ_MAX_CHAT_USER_NAME_SIZE) { // TODO allow only alphaneumeric characters
			aroop_txt_set_length(&greet_on_login, 0);
			aroop_txt_concat_string(&greet_on_login, "The name is too big\n");
			break;
		}
		// check availability
		async_try_login(&name, on_login_complete, chat->strm._ext.token);
	} while(0);
	if(!aroop_txt_is_empty(&greet_on_login))
		chat->strm.send(&chat->strm, &greet_on_login, 0);
	aroop_txt_destroy(&input);
	aroop_txt_destroy(&name);
	return 0;
}

static int chat_welcome_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) { // sanity check
		syslog(LOG_ERR, "BUG: no chat interface to welcome\n");
		return 0;
	}
	aroop_assert(chat->state == CHAT_CONNECTED);
	aroop_txt_t greet = {};
	aroop_txt_embeded_set_static_string(&greet, "Welcome to NginZ chat server\nLogin name?\n");
	chat->on_response_callback = on_login_data;
	chat->strm.send(&chat->strm, &greet, 0);
	return 0;
}

#if 0
static int chat_welcome_hookup(int signature, void*given) {
	hooks = (struct chat_hooks*)given;
	return 0;
}
#endif

static int chat_welcome_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "welcome", "chat", plugin_space, __FILE__, "It greets the new connection.\n");
}

int welcome_module_init() {
	aroop_txt_embeded_buffer(&greet_on_login, 128);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/_welcome");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_welcome_plug, chat_welcome_plug_desc);
#if 0
	aroop_txt_embeded_set_static_string(&plugin_space, "chatproto/hookup");
	pm_plug_bridge(&plugin_space, chat_welcome_hookup, chat_welcome_plug_desc);
#endif
}

int welcome_module_deinit() {
#if 0
	pm_unplug_bridge(0, chat_welcome_hookup);
#endif
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_welcome_plug);
	aroop_txt_destroy(&greet_on_login);
}


C_CAPSULE_END
