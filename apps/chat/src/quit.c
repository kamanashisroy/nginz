
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "streamio.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/user.h"
#include "chat/quit.h"

C_CAPSULE_START

static aroop_txt_t BYE = {};
static int chat_quit_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	broadcast_room_leave(chat);
	// chat->set_state(chat, CHAT_QUIT);
	chat->state |= CHAT_QUIT;
	logoff_user(chat);
	chat->strm.send(&chat->strm, &BYE, 0);
	return -1;
}

static int chat_quit_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "quit", "chat", plugin_space, __FILE__, "It quits the chat connection.\n");
}

int quit_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/quit");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_quit_plug, chat_quit_plug_desc);
	aroop_txt_embeded_set_static_string(&BYE, "BYE\n");
}

int quit_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_quit_plug);
}


C_CAPSULE_END
