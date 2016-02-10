
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "plugin.h"
#include "plugin_manager.h"
#include "net/chat/welcome.h"

C_CAPSULE_START

static int chat_welcome_plug(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_set_static_string(output, "Welcome to chat server\n");
	return 0;
}

static int chat_welcome_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "welcome", "chat", plugin_space, __FILE__, "It greets the new connection.\n");
}

int welcome_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/welcome");
	pm_plug_callback(&plugin_space, chat_welcome_plug, chat_welcome_plug_desc);
}

int welcome_module_deinit() {
	pm_unplug_callback(0, chat_welcome_plug);
}


C_CAPSULE_END
