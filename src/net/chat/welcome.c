
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginez_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/welcome.h"

C_CAPSULE_START

static int chat_welcome_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_interface*x = (struct chat_interface*)given;
	if(x->fd == -1) // sanity check
		return 0;
	aroop_txt_t greet = {};
	aroop_txt_embeded_set_static_string(&greet, "Welcome to NgineZ chat server\nLogin name?\n");
	send(x->fd, aroop_txt_to_string(&greet), aroop_txt_length(&greet), 0);
	return 0;
}

static int chat_welcome_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "welcome", "chat", plugin_space, __FILE__, "It greets the new connection.\n");
}

int welcome_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/welcome");
	composite_plug_bridge(chat_context_get(), &plugin_space, chat_welcome_plug, chat_welcome_plug_desc);
}

int welcome_module_deinit() {
	composite_unplug_bridge(chat_context_get(), 0, chat_welcome_plug);
}


C_CAPSULE_END
