
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_str2.h>
#include "nginz_config.h"
#include "plugin.h"
#include "log.h"
#include "streamio.h"
#include "scanner.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/hiddenjoin.h"

C_CAPSULE_START

static int chat_hiddenjoin_get_room_and_name(aroop_txt_t*request, aroop_txt_t*room, aroop_txt_t*name) {
	aroop_txt_t request_sandbox = {};
	aroop_txt_t token = {};
	//int reqlen = aroop_txt_length(request);
	//aroop_txt_embeded_stackbuffer(&request_sandbox, reqlen);
	//aroop_txt_concat(&request_sandbox, request);
	aroop_txt_embeded_stackbuffer_from_txt(&request_sandbox, request);
	scanner_next_token(&request_sandbox, &token); // chat/_hiddenjoin
	if(aroop_txt_is_empty(&token)) {
		return -1;
	}
	scanner_next_token(&request_sandbox, &token); // room
	if(aroop_txt_is_empty(&token)) {
		return -1;
	}
	aroop_txt_embeded_rebuild_copy_on_demand(room, &token); // needs cleanup
	aroop_txt_zero_terminate(room);
	scanner_next_token(&request_sandbox, &token); // name
	if(aroop_txt_is_empty(&token))
		return -1;
	aroop_txt_embeded_rebuild_copy_on_demand(name, &token); // needs cleanup
	aroop_txt_zero_terminate(name);
	return 0;
}

static int chat_hiddenjoin_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	aroop_txt_t room = {};
	aroop_txt_t name = {};
	chat->state |= CHAT_LOGGED_IN;
	do {
		if(aroop_txt_is_empty_magical(chat->request) || chat_hiddenjoin_get_room_and_name(chat->request, &room, &name)) {
			syslog(LOG_ERR, "Bug in the server code :(\n");
			break;
		}
		aroop_txt_destroy(&chat->name);
		aroop_txt_embeded_copy_deep(&chat->name, &name);
		//printf("doing hidden join to [%s] name [%s]\n", aroop_txt_to_string(&room), aroop_txt_to_string(&name));
		broadcast_room_join(chat, &room);
	} while(0);
	//printf("hiddenjoin complete\n");
	aroop_txt_destroy(&room);
	aroop_txt_destroy(&name);
	return 0;
}

static int chat_hiddenjoin_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "_hiddenjoin", "chat", plugin_space, __FILE__, "It responds to _hiddenjoin command for interprocess communication.\n");
}

int hiddenjoin_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/_hiddenjoin");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_hiddenjoin_plug, chat_hiddenjoin_plug_desc);
	return 0;
}

int hiddenjoin_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_hiddenjoin_plug);
	return 0;
}


C_CAPSULE_END
