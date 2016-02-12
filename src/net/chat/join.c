
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/join.h"

C_CAPSULE_START

static int chat_join_transfer(struct chat_connection*chat, aroop_txt_t*room, int pid) {
	// leave current chatroom
	broadcast_room_leave(chat);
	// remove it from listener
	event_loop_unregister_fd(chat->fd); // XXX do I need it ??
	aroop_txt_t cmd = {};
	aroop_txt_embeded_stackbuffer(&cmd, 128);
	aroop_txt_concat_string(&cmd, "chat/_hiddenjoin ");
	aroop_txt_concat(&cmd, room);
	aroop_txt_concat_string(&cmd, " ");
	aroop_txt_concat(&cmd, &chat->name);
	aroop_txt_concat_string(&cmd, "\n");
	aroop_txt_zero_terminate(&cmd);
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, pid);
	binary_pack_string(&bin, &cmd);
	int mypid = getpid();
	if(pid > mypid) {
		printf("transfering to %d ping\n", pid);
		pp_pingmsg(chat->fd, &bin);
	} else {
		printf("transfering to %d pong\n", pid);
		pp_pongmsg(chat->fd, &bin);
	}
	chat->state = CHAT_SOFT_QUIT; // quit the user from this process
	return 0;
}

static int chat_join_helper(struct chat_connection*chat, aroop_txt_t*room, int pid) {
	// check if it is same process
	printf("target pid=%d, my pid = %d\n", pid, getpid());
	if(pid != getpid()) {
		chat_join_transfer(chat, room, pid);
		return 0;
	}
	printf("assiging to room %s\n", aroop_txt_to_string(room));
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
	shotodol_scanner_next_token(&request_sandbox, &token);
	if(aroop_txt_is_empty(&token)) {
		return -1;
	}
	aroop_txt_embeded_rebuild_copy_on_demand(room, &token);
	aroop_txt_zero_terminate(room);
	return 0;
}

static int chat_join_search_pid(aroop_txt_t*room) {
	// remove trailing newline
	if(aroop_txt_is_empty(room)) {
		return -1;
	}

	// find the associated process to the room
	aroop_txt_t db_data = {};
	db_get(aroop_txt_to_string(room), &db_data);
	if(aroop_txt_is_empty(&db_data)) {
		aroop_txt_destroy(&db_data);
		return -1;
	}
	int pid = aroop_txt_to_int(&db_data);
	if(pid == 0)
		return -1;
	return pid;
}

static int chat_join_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(chat == NULL || chat->fd == -1) // sanity check
		return 0;
	aroop_txt_t join_info = {};
	aroop_txt_t room = {};
	int pid = -1;
	do {
		if(aroop_txt_is_empty_magical(&chat->name)) {
			aroop_txt_embeded_set_static_string(&join_info, "Please login first\n");
			break;
		}
		if(aroop_txt_is_empty_magical(chat->request) || chat_join_get_room(chat->request, &room) || (pid = chat_join_search_pid(&room)) == -1) {
			aroop_txt_embeded_set_static_string(&join_info, "The room is not avilable\n");
			break;
		}
		aroop_txt_embeded_stackbuffer(&join_info, 64);
		aroop_txt_printf(&join_info, "Switching to process %d\n", pid);
		chat_join_helper(chat, &room, pid);
	} while(0);
	send(chat->fd, aroop_txt_to_string(&join_info), aroop_txt_length(&join_info), 0);
	aroop_txt_destroy(&room);
	aroop_txt_destroy(&join_info);
	return 0;
}

static int chat_join_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "join", "chat", plugin_space, __FILE__, "It respons to join command.\n");
}

int join_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/join");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_join_plug, chat_join_plug_desc);
}

int join_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_join_plug);
}


C_CAPSULE_END
