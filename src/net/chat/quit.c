
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/user.h"
#include "net/chat/quit.h"

C_CAPSULE_START

static aroop_txt_t BYE = {};
static int chat_quit_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(chat == NULL || chat->fd == -1) // sanity check
		return 0;
	broadcast_room_leave(chat);
	// chat->set_state(chat, CHAT_QUIT);
	chat->state = CHAT_QUIT;
	logoff_user(&chat->name);
	send(chat->fd, aroop_txt_to_string(&BYE), aroop_txt_length(&BYE), 0);
	return -1;
}

static int chat_quit_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "quit", "chat", plugin_space, __FILE__, "It quits the chat connection.\n");
}

int quit_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/quit");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_quit_plug, chat_quit_plug_desc);
	aroop_txt_embeded_set_static_string(&BYE, "BYE\n");
}

int quit_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_quit_plug);
}


C_CAPSULE_END
