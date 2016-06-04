
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_str2.h>
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include <sys/socket.h>
#include "streamio.h"
#include "event_loop.h"
#include "binary_coder.h"
#include "scanner.h"
#include "lazy_call.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/broadcast.h"
#include "chat/join.h"
#include "chat/room.h"

C_CAPSULE_START

static int chat_join_transfer(struct chat_connection*chat, aroop_txt_t*room, int pid) {
	// leave current chatroom
	broadcast_room_leave(chat);
	// remove it from listener
	event_loop_unregister_fd(chat->strm.fd);
	aroop_txt_t cmd = {};
	aroop_txt_embeded_stackbuffer(&cmd, 128);
	aroop_txt_concat_string(&cmd, "chat/_hiddenjoin ");
	aroop_txt_concat(&cmd, room);
	aroop_txt_concat_string(&cmd, " ");
	aroop_txt_concat(&cmd, &chat->name);
	aroop_txt_concat_string(&cmd, "\n");
	aroop_txt_zero_terminate(&cmd);
	chat->strm.transfer_parallel(&chat->strm, pid, NGINZ_CHAT_PORT, &cmd);
	chat->state |= CHAT_SOFT_QUIT; // quit the user from this process
	chat->strm.close(&chat->strm);
	lazy_cleanup(chat);
	return 0;
}

static int chat_join_helper(struct chat_connection*chat, aroop_txt_t*room, int pid) {
	// check if it is same process
	//printf("target pid=%d, my pid = %d\n", pid, getpid());
	if(pid != getpid()) {
		chat_join_transfer(chat, room, pid);
		return 0;
	}
	//printf("assiging to room %s\n", aroop_txt_to_string(room));
	// otherwise assign to the room
	broadcast_room_join(chat, room);
	return 0;
}

static int chat_join_get_room(aroop_txt_t*request, aroop_txt_t*room) {
	aroop_txt_t request_sandbox = {};
	aroop_txt_t token = {};
	int reqlen = aroop_txt_length(request);
	aroop_txt_embeded_stackbuffer(&request_sandbox, reqlen);
	aroop_txt_concat(&request_sandbox, request);
	scanner_next_token(&request_sandbox, &token);
	if(aroop_txt_is_empty(&token)) {
		return -1;
	}
	aroop_txt_embeded_rebuild_copy_on_demand(room, &token); // needs cleanup
	aroop_txt_zero_terminate(room);
	return 0;
}

static int on_asyncchat_room_pid(aroop_txt_t*bin, aroop_txt_t*unused) {
	aroop_assert(!aroop_txt_is_empty_magical(bin));
	// 0 = srcpid, 1 = command, 2 = token, 3 = success, 4 = key, 5 = newvalue
	//syslog(LOG_NOTICE, "------------ ..............  \n");
	int cb_token = 0;
	aroop_txt_t pidstr = {};
	aroop_txt_t join_info = {};
	aroop_txt_t room_key = {};
	aroop_txt_t room = {};
	binary_unpack_int(bin, 2, &cb_token); // id/token
	binary_unpack_string(bin, 4, &room_key);
	//syslog(LOG_NOTICE, "Joining ..............  %d to %s\n", cb_token, aroop_txt_to_string(&room_key));
	chat_room_convert_room_from_room_pid_key(&room_key, &room); // needs cleanup
	//syslog(LOG_NOTICE, "Joining ..............  %d to %s\n", cb_token, aroop_txt_to_string(&room));
	binary_unpack_string(bin, 5, &pidstr); // needs cleanup
	aroop_txt_t pidstrdup = {};
	aroop_txt_embeded_stackbuffer(&pidstrdup, 32);
	aroop_txt_concat(&pidstrdup, &pidstr);
	aroop_txt_zero_terminate(&pidstrdup);
	struct chat_connection*chat = chat_api_get()->get(cb_token); // needs cleanup
	do {
		if(!chat) {
			syslog(LOG_ERR, "Join failed, chat object not found %d\n", cb_token);
			break;
		}
		int pid = aroop_txt_to_int(&pidstrdup);
		//syslog(LOG_NOTICE, "Joining ..............  %d to %s : %d\n", cb_token, aroop_txt_to_string(&room), pid);
		if(pid <= 0 || aroop_txt_is_empty(&room)) {
			// say room not found
			aroop_txt_embeded_set_static_string(&join_info, "The room is not avilable\n");
			break;
		}
		aroop_txt_embeded_stackbuffer(&join_info, 64);
		aroop_txt_printf(&join_info, "Trying ...(%d)\n", pid);
		chat->strm.send(&chat->strm, &join_info, MSG_MORE);
		aroop_txt_set_length(&join_info, 0);
		chat_join_helper(chat, &room, pid);
	} while(0);
	if(!aroop_txt_is_empty(&join_info)) {
		chat->strm.send(&chat->strm, &join_info, 0);
	}
	aroop_txt_destroy(&pidstr);
	aroop_txt_destroy(&join_info);
	aroop_txt_destroy(&room_key);
	aroop_txt_destroy(&room);
	if(chat)
		OPPUNREF(chat); // cleanup
	return 0;
}

static int chat_join_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	aroop_txt_t join_info = {};
	aroop_txt_t room = {};
	aroop_txt_t room_lookup_hook = {};
	aroop_txt_embeded_set_static_string(&room_lookup_hook, "on/asyncchat/room/pid");
	do {
		if(aroop_txt_is_empty_magical(&chat->name)) {
			aroop_txt_embeded_set_static_string(&join_info, "Please login first\n");
			break;
		}
		if(aroop_txt_is_empty_magical(chat->request) || chat_join_get_room(chat->request, &room) || (chat_room_get_pid(&room, chat->strm._ext.token, &room_lookup_hook)) == -1) {
			aroop_txt_embeded_set_static_string(&join_info, "The room is not avilable\n");
			break;
		}
		aroop_txt_embeded_stackbuffer(&join_info, 64);
		aroop_txt_printf(&join_info, "Trying ...\n");
		chat->strm.send(&chat->strm, &join_info, MSG_MORE);
		aroop_txt_set_length(&join_info, 0);
	} while(0);
	if(!aroop_txt_is_empty(&join_info)) {
		chat->strm.send(&chat->strm, &join_info, 0);
	}
	aroop_txt_destroy(&room);
	aroop_txt_destroy(&join_info);
	return 0;
}

static int chat_join_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "join", "chat", plugin_space, __FILE__, "It respons to join command.\n");
}

static int on_asyncchat_room_pid_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "react to callback", "asyncchat", plugin_space, __FILE__, "It asynchronously responds to join request.\n");
}

int join_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/join");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_join_plug, chat_join_plug_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "on/asyncchat/room/pid");
	pm_plug_callback(&plugin_space, on_asyncchat_room_pid, on_asyncchat_room_pid_desc);
	return 0;
}

int join_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_join_plug);
	pm_unplug_callback(0, on_asyncchat_room_pid);
	return 0;
}


C_CAPSULE_END
