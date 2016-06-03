
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "log.h"
#include "streamio.h"
#include "scanner.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/user.h"
#include "chat/broadcast.h"
#include "chat/private_message.h"

C_CAPSULE_START

static int chat_private_message_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	if(!(chat->state & CHAT_IN_ROOM)) { // we cannot send private message if not in room
		// TODO say appology
		return 0;
	}
	// get the destined user
	aroop_txt_t other = {};
	aroop_txt_t sandbox = {};
	aroop_txt_embeded_stackbuffer_from_txt(&sandbox, chat->request);
	scanner_next_token(&sandbox, &other); // destined user
	if(aroop_txt_is_empty(&other) || aroop_txt_is_empty(&sandbox)) { // /pm cannot do anything without parameter
		return 0;
	}
	aroop_txt_t pm = {};
	aroop_txt_embeded_stackbuffer(&pm, 32 + NGINZ_MAX_CHAT_USER_NAME_SIZE + NGINZ_MAX_CHAT_MSG_SIZE);
	aroop_txt_concat(&pm, &chat->name); // show the message sender name
	aroop_txt_concat_char(&pm, '~');
	aroop_txt_concat(&pm, &sandbox); // show the message
	broadcast_private_message(chat, &other, &pm);
	return 0;
}

static int chat_private_message_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "pm", "chat", plugin_space, __FILE__, "/pm username your private message.\n");
}

int private_message_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/pm");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_private_message_plug, chat_private_message_plug_desc);
}

int private_message_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_private_message_plug);
}


C_CAPSULE_END
