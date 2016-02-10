
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/welcome.h"

C_CAPSULE_START

aroop_txt_t greet_on_login = {};
static int on_login_data(struct chat_connection*chat, aroop_txt_t*answer) {
	// TODO authenticate
	if(aroop_txt_is_empty_magical(answer))
		return 0;
	aroop_txt_set_length(&greet_on_login, 0);
	aroop_txt_concat_string(&greet_on_login, "Welcome ");
	aroop_txt_concat(&greet_on_login, answer);
	aroop_txt_concat_char(&greet_on_login, '!');
	aroop_txt_concat_char(&greet_on_login, '\n');
	send(chat->fd, aroop_txt_to_string(&greet_on_login), aroop_txt_length(&greet_on_login), 0);
	chat->on_answer = NULL;
	return 0;
}

static int chat_welcome_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(chat == NULL || chat->fd == -1) // sanity check
		return 0;
	aroop_txt_t greet = {};
	aroop_txt_embeded_set_static_string(&greet, "Welcome to NginZ chat server\nLogin name?\n");
	chat->on_answer = on_login_data;
	send(chat->fd, aroop_txt_to_string(&greet), aroop_txt_length(&greet), 0);
	return 0;
}

static int chat_welcome_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "welcome", "chat", plugin_space, __FILE__, "It greets the new connection.\n");
}

int welcome_module_init() {
	aroop_txt_embeded_buffer(&greet_on_login, 128);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/welcome");
	composite_plug_bridge(chat_context_get(), &plugin_space, chat_welcome_plug, chat_welcome_plug_desc);
}

int welcome_module_deinit() {
	composite_unplug_bridge(chat_context_get(), 0, chat_welcome_plug);
	aroop_txt_destroy(&greet_on_login);
}


C_CAPSULE_END
