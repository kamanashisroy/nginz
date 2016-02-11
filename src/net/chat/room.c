
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "db.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/room.h"

C_CAPSULE_START

static const char*ROOM_KEY = "rooms";
static int chat_room_lookup_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(chat == NULL || chat->fd == -1) // sanity check
		return 0;
	aroop_txt_t room_info = {};
	aroop_txt_t db_data = {};
	db_get(ROOM_KEY, &db_data);
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

/****************************************************/
/****** Default room setup **************************/
/****************************************************/
static const char default_rooms[][32] = {"ONE", "TWO", "THREE", "FOUR", "FIVE"};
static int default_room_setup() {
	if(!is_master()) // we only start it in the master
		return 0;
	aroop_txt_t roomstr = {};
	aroop_txt_embeded_stackbuffer(&roomstr, sizeof(default_rooms)); 
	int i = 0;
	int room_count = sizeof(default_rooms)/sizeof(*default_rooms);
	for(i=0;i<room_count;i++) {
		aroop_txt_concat_string(&roomstr, default_rooms[i]);
		aroop_txt_concat_char(&roomstr, ' '); // add space to separate words
	}
	aroop_txt_shift(&roomstr, -1);// trim the last space
	aroop_txt_zero_terminate(&roomstr);

	db_set(ROOM_KEY, aroop_txt_to_string(&roomstr));
	return 0;
}

static int internal_child_count = 0;
static int default_room_fork_child_after_callback(aroop_txt_t*input, aroop_txt_t*output) {
	const char*myroom = default_rooms[internal_child_count];
	int pid = getpid();
	aroop_txt_t pidstr = {};
	aroop_txt_embeded_stackbuffer(&pidstr, 32);
	aroop_txt_printf(&pidstr, "%d", pid);
	aroop_txt_zero_terminate(&pidstr);
	printf("we are assigning room(%s) to a process(%s)\n", myroom, aroop_txt_to_string(&pidstr));
	db_set(myroom, aroop_txt_to_string(&pidstr));
	aroop_txt_t room_name = {};
	aroop_txt_embeded_copy_string(&room_name, myroom);
	broadcast_add_room(&room_name);
	aroop_txt_destroy(&room_name);
	internal_child_count++;
	//aroop_txt_destroy(&pidstr);
	return 0;
}

static int default_room_fork_callback_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "room", "fork", plugin_space, __FILE__, "It assigns a room to a worker process.\n");
}

int room_module_init() {
	default_room_setup();
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/rooms");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_room_lookup_plug, chat_room_lookup_plug_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_callback(&plugin_space, default_room_fork_child_after_callback, default_room_fork_callback_desc);
}

int room_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_room_lookup_plug);
	pm_unplug_callback(0, default_room_fork_child_after_callback);
}


C_CAPSULE_END
