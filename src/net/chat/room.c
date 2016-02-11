
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/room.h"

C_CAPSULE_START

const char*room_key = "rooms";
aroop_txt_t room_info = {};
static int chat_room_lookup_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(chat == NULL || chat->fd == -1) // sanity check
		return 0;
	aroop_txt_t room_info = {};
	aroop_txt_t db_data = {};
	db_get(room_key, &db_data);
	if(aroop_txt_is_empty(&db_data)) {
		aroop_txt_embeded_set_static_string(&room_info, "There is no room\n");
	} else {
		int len = aroop_txt_length(&db_data)+4;
		aroop_txt_embeded_stackbuffer(&room_info, len);
		aroop_txt_concat(&room_info, &db_data);
		aroop_txt_concat_char(&room_info, '\n');
	}
	send(chat->fd, aroop_txt_to_string(&room_info), aroop_txt_length(&room_info), 0);
	aroop_txt_destroy(&room_info);
	return 0;
}

static int chat_room_lookup_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "room", "chat", plugin_space, __FILE__, "It respons to room command.\n");
}

static int room_setup() {
	if(!is_master()) // we only start it in the master
		return 0;
	db_save(room_key, "ONE TWO THREE");
	return 0;
}

int room_module_init() {
	room_setup();
	aroop_txt_embeded_buffer(&room_info, 128);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/rooms");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_room_lookup_plug, chat_room_lookup_plug_desc);
}

int room_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_room_lookup_plug);
	aroop_txt_destroy(&room_info);
}


C_CAPSULE_END
