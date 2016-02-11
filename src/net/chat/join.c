
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/join.h"

C_CAPSULE_START

aroop_txt_t join_info = {};
static int chat_join_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(chat == NULL || chat->fd == -1) // sanity check
		return 0;
	aroop_txt_t join_info = {};
	aroop_txt_t db_data = {};
	do {
		if(chat->request == NULL) {
			aroop_txt_embeded_set_static_string(&join_info, "There is no such room\n");
			break;
		}
		// remove trailing newline
		aroop_txt_t request_sandbox = {};
		aroop_txt_t room = {};
		int reqlen = aroop_txt_length(chat->request);
		aroop_txt_embeded_stackbuffer(&request_sandbox, reqlen);
		aroop_txt_concat(&request_sandbox, chat->request);
		shotodol_scanner_next_token(&request_sandbox, &room);
		aroop_txt_zero_terminate(&room);
		if(aroop_txt_is_empty(&room)) {
			aroop_txt_embeded_set_static_string(&join_info, "There is no such room\n");
			break;
		}
		db_get(aroop_txt_to_string(&room), &db_data);
		if(aroop_txt_is_empty(&db_data)) {
			aroop_txt_embeded_set_static_string(&join_info, "There is no such room\n");
			break;
		}
		int len = aroop_txt_length(&db_data)+32;
		aroop_txt_embeded_stackbuffer(&join_info, len);
		aroop_txt_concat_string(&join_info, "Switching to process ");
		aroop_txt_concat(&join_info, &db_data);
		aroop_txt_concat_char(&join_info, '\n');
	} while(0);
	send(chat->fd, aroop_txt_to_string(&join_info), aroop_txt_length(&join_info), 0);
	aroop_txt_destroy(&join_info);
	aroop_txt_destroy(&db_data);
	return 0;
}

static int chat_join_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "join", "chat", plugin_space, __FILE__, "It respons to join command.\n");
}

int join_module_init() {
	aroop_txt_embeded_buffer(&join_info, 128);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/join");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_join_plug, chat_join_plug_desc);
}

int join_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_join_plug);
	aroop_txt_destroy(&join_info);
}


C_CAPSULE_END
