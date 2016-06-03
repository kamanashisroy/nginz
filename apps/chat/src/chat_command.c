
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "scanner.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "protostack.h"
#include "streamio.h"
#include "chat.h"
#include "chat/chat_command.h"

C_CAPSULE_START



static int command_hook(struct chat_connection*chat, aroop_txt_t*given_request) {
	int ret = 0;
	// make a copy
	aroop_txt_t request = {};
	aroop_txt_embeded_txt_copy_shallow(&request, given_request); // needs cleanup
	aroop_txt_shift(&request, 1); // skip '/' before command
	if(aroop_txt_char_at(&request, 0) == '_') { // do now allow hidden commands
		aroop_txt_destroy(&request);
		return 0;
	}

	aroop_assert(chat->request == NULL);
	// get the command token
	aroop_txt_t ctoken = {};
	scanner_next_token(&request, &ctoken);
	do {
		if(aroop_txt_is_empty_magical(&ctoken)) {
			// we cannot handle the data
			ret = -1;
			break;
		}
		aroop_txt_shift(&request, 1); // skip SPACE after command
		chat->request = &request;
		aroop_txt_t plugin_space = {};
		aroop_txt_embeded_stackbuffer(&plugin_space, 64);
		aroop_txt_concat_string(&plugin_space, "chat/");
		aroop_txt_concat(&plugin_space, &ctoken);
		ret = composite_plugin_bridge_call(chat_plugin_manager_get(), &plugin_space, CHAT_SIGNATURE, chat);
		aroop_txt_destroy(&plugin_space);
		chat->request = NULL; // cleanup
	} while(0);
	// cleanup
	aroop_txt_destroy(&ctoken);
	aroop_txt_destroy(&request);
	return ret;
}

int chat_command_module_init() {
	chat_api_get()->on_command = command_hook;
	return 0;
}

int chat_command_module_deinit() {
	return 0;
}

