
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "log.h"
#include "db.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "streamio.h"
#include "scanner.h"
#include "binary_coder.h"
#include "parallel/pipeline.h"
#include "parallel/async_request.h"
#include "async_db.h"
#include "async_db_master.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/room.h"
#include "chat/broadcast.h"
#include "chat/room_master.h"

C_CAPSULE_START

static const char*ROOM_PID_KEY = "room/pid/";
static const char*ROOM_USER_KEY = "room/user/";

static int chat_room_get_user_count(aroop_txt_t*my_room) {
	if(aroop_txt_is_empty(my_room)) {
		return -1;
	}
	// get the number of users in that room
	aroop_txt_t db_room_key = {};
	aroop_txt_embeded_stackbuffer(&db_room_key, 128);
	aroop_txt_concat_string(&db_room_key, ROOM_USER_KEY);
	aroop_txt_concat(&db_room_key, my_room);
	aroop_txt_t countstr = {};
	noasync_db_get(&db_room_key, &countstr); // needs cleanup
	aroop_txt_t cpystr = {};
	aroop_txt_embeded_stackbuffer(&cpystr, 32);
	aroop_txt_concat(&cpystr, &countstr);
	aroop_txt_zero_terminate(&cpystr);
	int retval = aroop_txt_to_int(&cpystr);
	aroop_txt_destroy(&countstr); // cleanup
	return retval;
}

static int chat_room_describe_helper(aroop_txt_t*my_room, aroop_txt_t*room_info) {
	int count = chat_room_get_user_count(my_room);
	if(count < 0)
		count = 0; // fixup
	// prepare output
	aroop_txt_concat_char(room_info, '\t');
	aroop_txt_concat_char(room_info, '*');
	aroop_txt_concat_char(room_info, ' ');
	aroop_txt_concat(room_info, my_room);
	if(count) {
		aroop_txt_t count_str = {};
		aroop_txt_embeded_stackbuffer(&count_str, 32);
		aroop_txt_printf(&count_str, " (%d)", count);
		aroop_txt_concat(room_info, &count_str);
	}
	aroop_txt_concat_char(room_info, '\n');
	return 0;
}

static int chat_room_describe(aroop_txt_t*roomstr, aroop_txt_t*room_info) {
	aroop_txt_t next = {};
	aroop_txt_concat_string(room_info, "Active rooms are:\n");
	while(1) {
		scanner_next_token(roomstr, &next); // needs cleanup
		if(aroop_txt_is_empty(&next)) {
			break;
		}
		chat_room_describe_helper(&next, room_info);
		aroop_txt_destroy(&next); // cleanup
	}
	aroop_txt_concat_string(room_info, "end of list.\n");
	aroop_txt_destroy(&next);
	return 0;
}

#define ROOM_KEY "rooms"

static int on_async_room_call_master(aroop_txt_t*bin, aroop_txt_t*output) {
	aroop_assert(!aroop_txt_is_empty_magical(bin));
	int srcpid = 0;
	int cb_token = 0;
	aroop_txt_t cb_hook = {};
	// 0 = srcpid, 1 = command, 2 = token, 3 = cb_hook, 4 = key, 5 = newval, 6 = oldval
	binary_unpack_int(bin, 0, &srcpid);
	binary_unpack_int(bin, 2, &cb_token);
	binary_unpack_string(bin, 3, &cb_hook); // needs cleanup
	aroop_txt_t info = {};
	aroop_txt_t room_info = {};
	aroop_txt_t key = {};
	aroop_txt_embeded_set_static_string(&key, ROOM_KEY);
	noasync_db_get(&key, &info); // needs cleanup
	if(!aroop_txt_is_empty(&info)) {
		const int len = aroop_txt_length(&info)*32 + 64;
		aroop_txt_embeded_stackbuffer(&room_info, len); // FIXME too many chatroom will cause stack overflow ..
		chat_room_describe(&info, &room_info);
	}
	//syslog(LOG_ERR, "rooms:%s\n", aroop_txt_to_string(&room_info));
	aroop_txt_t*args[2] = {&room_info, NULL};
	async_pm_reply_worker(srcpid, cb_token, &cb_hook, 1, args);
	aroop_txt_destroy(&info);
	aroop_txt_destroy(&room_info);
	return 0;
}

static int on_asyncchat_rooms_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "on/asyncchat/rooms", "asyncchat", plugin_space, __FILE__, "It responds to asynchronous response from db.\n");
}


/****************************************************/
/****** Default room setup **************************/
/****************************************************/
static const char default_rooms[][32] = {"ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN"};
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
	aroop_txt_t key = {};
	aroop_txt_embeded_set_static_string(&key, ROOM_KEY);
	async_db_compare_and_swap(-1, NULL, &key, &roomstr, NULL);
	return 0;
}

static int chat_room_add_helper(int index) {
	const char*myroom = default_rooms[index];
	int pid = getpid();
	//printf("we are assigning room(%s) to a process(%d)\n", myroom, pid);
	aroop_txt_t room_name = {};
	aroop_txt_embeded_stackbuffer(&room_name, 128);
	aroop_txt_concat_string(&room_name, myroom); 

	/** Save the pid for the room in the database */
	aroop_txt_t db_room_key = {};
	aroop_txt_embeded_stackbuffer(&db_room_key, 128);
	aroop_txt_concat_string(&db_room_key, ROOM_PID_KEY);
	aroop_txt_concat(&db_room_key, &room_name);
	aroop_txt_zero_terminate(&db_room_key);
	async_db_set_int(-1, NULL, &db_room_key, pid);

	/** create the room constructs */
	broadcast_add_room(&room_name);
	return 0;
}

static int internal_child_count = -1;
static int default_room_fork_before_callback(int signature, void*unused) {
	aroop_assert(NGINZ_PIPELINE_SIGNATURE == signature);
	internal_child_count++; // add default indexes
	return 0;
}
static int default_room_fork_child_after_callback(int signature, void*unused) {
	aroop_assert(NGINZ_PIPELINE_SIGNATURE == signature);
	chat_room_add_helper(internal_child_count); // add default indexes
	//chat_room_add_helper(internal_child_count++); // add default indexes
	return 0;
}

static int default_room_fork_callback_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "room", "fork", plugin_space, __FILE__, "It assigns a room to a worker process.\n");
}

int room_master_module_init() {
	aroop_assert(is_master());
	default_room_setup();
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, ON_ASYNC_ROOM_CALL);
	pm_plug_callback(&plugin_space, on_async_room_call_master, on_asyncchat_rooms_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/before");
	pm_plug_bridge(&plugin_space, default_room_fork_before_callback, default_room_fork_callback_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_bridge(&plugin_space, default_room_fork_child_after_callback, default_room_fork_callback_desc);
	return 0;
}

int room_master_module_deinit() {
	aroop_assert(is_master());
	pm_unplug_bridge(0, default_room_fork_child_after_callback);
	pm_unplug_bridge(0, default_room_fork_before_callback);
	pm_unplug_callback(0, on_async_room_call_master);
	return 0;
}


C_CAPSULE_END
