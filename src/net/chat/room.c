
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "log.h"
#include "db.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/room.h"
#include "net/chat/room_master.h"

C_CAPSULE_START

static const char*ROOM_PID_KEY = "room/pid/";
static const char*ROOM_USER_KEY = "room/user/";
int chat_room_convert_room_from_room_pid_key(aroop_txt_t*room_key, aroop_txt_t*room) {
	aroop_assert(room_key);
	aroop_assert(room);
	aroop_txt_embeded_buffer(room, 64); // XXX it can break any time !
	aroop_txt_concat(room, room_key);
	int len = strlen(ROOM_PID_KEY);
	aroop_txt_shift(room, len);
	return 0;
}

int chat_room_set_user_count(aroop_txt_t*my_room, int user_count) {
	if(aroop_txt_is_empty(my_room)) {
		return -1;
	}
	// get the number of users in that room
	aroop_txt_t db_room_key = {};
	aroop_txt_embeded_stackbuffer(&db_room_key, 128);
	aroop_txt_concat_string(&db_room_key, ROOM_USER_KEY);
	aroop_txt_concat(&db_room_key, my_room);
	aroop_txt_zero_terminate(&db_room_key);
	async_db_set_int(-1, NULL, &db_room_key, user_count);
	return 0;
}

int chat_room_get_pid(aroop_txt_t*my_room, int token, aroop_txt_t*callback_hook) {
	if(aroop_txt_is_empty(my_room)) {
		return -1;
	}
	// get the number of users in that room
	aroop_txt_t db_room_key = {};
	aroop_txt_embeded_stackbuffer(&db_room_key, 128);
	aroop_txt_concat_string(&db_room_key, ROOM_PID_KEY);
	aroop_txt_concat(&db_room_key, my_room);
	aroop_txt_zero_terminate(&db_room_key);
	async_db_get(token, callback_hook, &db_room_key);
	return 0;
}

static int chat_room_lookup_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	aroop_txt_t on_async_reply = {};
	aroop_txt_embeded_set_static_string(&on_async_reply, ON_ASYNC_ROOM_REPLY);
	aroop_txt_t on_async_request = {};
	aroop_txt_embeded_set_static_string(&on_async_request, ON_ASYNC_ROOM_CALL);
	aroop_txt_t*empty[1] = {NULL};
	async_pm_call_master(chat->strm._ext.token, &on_async_reply, &on_async_request, empty);
	return 0;
}

static int on_asyncchat_rooms(aroop_txt_t*bin, aroop_txt_t*output) {
	aroop_assert(!aroop_txt_is_empty_magical(bin));
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = success, 5 = info
	int cb_token = 0;
	aroop_txt_t info = {};
	binary_unpack_int(bin, 3, &cb_token); // id/token
	binary_unpack_string(bin, 5, &info); // needs cleanup
	struct chat_connection*chat = chat_api_get()->get(cb_token); // needs cleanup
	do {
		if(!chat) {
			syslog(LOG_ERR, "Error, could not find the chat user:%d\n", cb_token);
			break;
		}
		if(aroop_txt_is_empty(&info)) {
			aroop_txt_embeded_set_static_string(&info, "There is no room\n");
		}
		chat->strm.send(&chat->strm, &info, 0);
	} while(0);
	aroop_txt_destroy(&info); // cleanup
	OPPUNREF(chat); // cleanup
	return 0;
}

static int on_asyncchat_rooms_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "on/asyncchat/rooms", "asyncchat", plugin_space, __FILE__, "It responds to asynchronous response from db.\n");
}


static int chat_room_lookup_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "room", "chat", plugin_space, __FILE__, "It respons to room command.\n");
}
int room_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/rooms");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_room_lookup_plug, chat_room_lookup_plug_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, ON_ASYNC_ROOM_REPLY);
	pm_plug_callback(&plugin_space, on_asyncchat_rooms, on_asyncchat_rooms_desc);
	if(is_master())
		room_master_module_init();
}

int room_module_deinit() {
	if(is_master())
		room_master_module_deinit();
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_room_lookup_plug);
	pm_unplug_callback(0, on_asyncchat_rooms);
}


C_CAPSULE_END
