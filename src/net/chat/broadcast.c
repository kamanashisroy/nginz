
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/broadcast.h"

C_CAPSULE_START

static int broadcast_callback(struct chat_connection*chat, aroop_txt_t*msg) {
	printf("TODO broadcast %s\n", aroop_txt_to_string(msg));
}

int broadcast_assign_to_room(struct chat_connection*chat, aroop_txt_t*room) {
	chat->on_broadcast = broadcast_callback;
	return 0;
}

int broadcast_module_init() {
	// TODO create room list
}
int broadcast_module_deinit() {
	// TODO destroy room list
}

C_CAPSULE_END

