
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/join.h"

C_CAPSULE_START

aroop_txt_t join_info = {};
static int chat_join_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(chat == NULL || chat->fd == -1) // sanity check
		return 0;
	aroop_txt_t join_info = {};
	aroop_txt_embeded_set_static_string(&join_info, "There is no join\n");
	send(chat->fd, aroop_txt_to_string(&join_info), aroop_txt_length(&join_info), 0);
	return 0;
}

static int chat_join_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "join", "chat", plugin_space, __FILE__, "It respons to join command.\n");
}

int join_module_init() {
	aroop_txt_embeded_buffer(&join_info, 128);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/join");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_join_plug, chat_join_plug_desc);
}

int join_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_join_plug);
	aroop_txt_destroy(&join_info);
}


C_CAPSULE_END
