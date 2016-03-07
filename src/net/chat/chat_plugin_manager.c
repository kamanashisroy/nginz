
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/chat_factory.h"
#include "net/chat/chat_accept.h"
#include "net/chat/chat_command.h"
#include "net/chat/welcome.h"
#include "net/chat/room.h"
#include "net/chat/hiddenjoin.h"
#include "net/chat/join.h"
#include "net/chat/broadcast.h"
#include "net/chat/leave.h"
#include "net/chat/quit.h"
#include "net/chat/private_message.h"
#include "net/chat/uptime.h"
#include "net/chat/version.h"

C_CAPSULE_START

static struct composite_plugin*chat_plug = NULL;

NGINZ_INLINE struct composite_plugin*chat_plugin_manager_get() {
	return chat_plug;
}

static int chat_plugin_command(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 1024);
	composite_plugin_visit_all(chat_plugin_manager_get(), plugin_command_helper, output);
	return 0;
}

static int chat_plugin_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "chatplugin", "shake", plugin_space, __FILE__, "It dumps the avilable plugins\n");
}

static int chat_help_plug_helper(
	int category
	, aroop_txt_t*plugin_space
	, int(*callback)(aroop_txt_t*input, aroop_txt_t*output)
	, int(*bridge)(int signature, void*x)
	, struct composite_plugin*inner
	, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)
	, void*visitor_data
) {
	aroop_txt_t prefix = {};
	aroop_txt_t*output = (aroop_txt_t*)visitor_data;
	aroop_txt_t xdesc = {};
	aroop_txt_embeded_rebuild_copy_shallow(&prefix, plugin_space); // needs destroy
	aroop_txt_set_length(&prefix, 5);
	do {
		if(!aroop_txt_equals_static(&prefix, "chat/"))
			break;
		if(aroop_txt_char_at(&prefix, 5) == '_') // hide the hidden commands
			break;
		desc(plugin_space, &xdesc);
		aroop_txt_concat(output, &xdesc);
	} while(0);
	aroop_txt_destroy(&xdesc); // cleanup
	aroop_txt_destroy(&prefix); // cleanup
}

static int chat_help_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	if(!chat_plugin_manager_get()) { // sanity check
		return 0;
	}
	aroop_txt_t output = {};
	aroop_txt_embeded_stackbuffer(&output, 1024);
	composite_plugin_visit_all(chat_plugin_manager_get(), chat_help_plug_helper, &output);
	chat->strm.send(&chat->strm, &output, 0);
	return 0;
}

static int chat_help_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "help", "chat", plugin_space, __FILE__, "It shows available commands.\n");
}

int chat_plugin_manager_module_init() {
	aroop_assert(chat_plug == NULL);
	chat_plug = composite_plugin_create();
	chat_factory_module_init();
	user_module_init();
	welcome_module_init();
	broadcast_module_init(); // XXX we have to load broadcast module before room module
	room_module_init();
	join_module_init();
	hiddenjoin_module_init();
	leave_module_init();
	quit_module_init();
	private_message_module_init();
	uptime_module_init();
	version_module_init();
	chat_profiler_module_init();
	chat_command_module_init();
	chat_accept_module_init();
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/help");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_help_plug, chat_help_plug_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/chatplugin");
	pm_plug_callback(&plugin_space, chat_plugin_command, chat_plugin_command_desc);
}

int chat_plugin_manager_module_deinit() {
	chat_accept_module_deinit();
	chat_command_module_deinit();
	chat_profiler_module_deinit();
	version_module_deinit();
	uptime_module_deinit();
	private_message_module_deinit();
	quit_module_deinit();
	leave_module_deinit();
	hiddenjoin_module_deinit();
	join_module_deinit();
	room_module_deinit();
	broadcast_module_deinit();
	welcome_module_deinit();
	user_module_deinit();
	chat_factory_module_deinit();
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_help_plug);
	composite_plugin_destroy(chat_plug);
}


C_CAPSULE_END
