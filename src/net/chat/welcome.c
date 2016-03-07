
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

aroop_txt_t greet_on_login = {};
static int on_asyncchat_login_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	aroop_assert(!aroop_txt_is_empty_magical(bin));
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = success, 5 = key, 6 = newvalue
	int cb_token = 0;
	int success = 0;
	aroop_txt_t name = {};
	binary_unpack_int(bin, 3, &cb_token); // id/token
	binary_unpack_int(bin, 4, &success);
	binary_unpack_string(bin, 6, &name); // needs cleanup
	//syslog(LOG_NOTICE, "[token%d]received:[value:%s]", cb_token, aroop_txt_to_string(&name));
	/*if(!aroop_txt_is_empty(&name)) {
		aroop_txt_shift(&name, sizeof(USER_PREFIX));
	}*/
	if(aroop_txt_is_empty_magical(&name)) {
		syslog(LOG_ERR, "Error, we do not know user name\n");
		aroop_txt_destroy(&name); // cleanup
		return 0;
	}
	struct chat_connection*chat = chat_api_get()->get(cb_token); // needs cleanup
	if(!chat) {
		syslog(LOG_ERR, "Error, could not find the logged in user\n");
		aroop_txt_destroy(&name); // cleanup
		return 0;
	}
	if(success) {
		// save name
		aroop_txt_embeded_copy_deep(&chat->name, &name);
		// say welcome
		aroop_txt_set_length(&greet_on_login, 0);
		aroop_txt_concat_string(&greet_on_login, "Welcome ");
		aroop_txt_concat(&greet_on_login, &name);
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
	aroop_txt_destroy(&name); // cleanup
	return 0;
}


static int on_asyncchat_login_hook_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "-", "chatuser", plugin_space, __FILE__, "It helps chat user to collect the hooks.\n");
}

static int on_asyncchat_login(struct chat_connection*chat, aroop_txt_t*answer) {
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
		aroop_txt_t login_hook = {};
		aroop_txt_embeded_set_static_string(&login_hook, "on/asyncchat/login");
		async_try_login(&name, chat->strm._ext.token, &login_hook);
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
	chat->on_response_callback = on_asyncchat_login;
	chat->strm.send(&chat->strm, &greet, 0);
	return 0;
}

static int chat_welcome_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "welcome", "chat", plugin_space, __FILE__, "It greets the new connection.\n");
}

int welcome_module_init() {
	aroop_txt_embeded_buffer(&greet_on_login, 128);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/_welcome");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_welcome_plug, chat_welcome_plug_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "on/asyncchat/login");
	pm_plug_callback(&plugin_space, on_asyncchat_login_hook , on_asyncchat_login_hook_desc);
}

int welcome_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_welcome_plug);
	pm_unplug_callback(0, on_asyncchat_login_hook);
	aroop_txt_destroy(&greet_on_login);
}


C_CAPSULE_END
