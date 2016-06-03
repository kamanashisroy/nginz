
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "streamio.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/leave.h"

C_CAPSULE_START

static int chat_leave_plug(int signature, void*given) {
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	broadcast_room_leave(chat);
	aroop_assert(!(chat->state & CHAT_IN_ROOM));
	return 0;
}

static int chat_leave_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "leave", "chat", plugin_space, __FILE__, "It disconnects user from the room.\n");
}

int leave_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/leave");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_leave_plug, chat_leave_plug_desc);
}
int leave_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_leave_plug);
}

C_CAPSULE_END

