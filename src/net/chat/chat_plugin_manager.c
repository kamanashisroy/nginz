
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/welcome.h"
#include "net/chat/room.h"
#include "net/chat/join.h"
#include "net/chat/broadcast.h"
#include "net/chat/leave.h"
#include "net/chat/quit.h"

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

int chat_plugin_manager_module_init() {
	aroop_assert(chat_plug == NULL);
	chat_plug = composite_plugin_create();
	welcome_module_init();
	broadcast_module_init(); // XXX we have to load broadcast module before room module
	room_module_init();
	join_module_init();
	leave_module_init();
	quit_module_init();
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/chatplugin");
	pm_plug_callback(&plugin_space, chat_plugin_command, chat_plugin_command_desc);
}

int chat_plugin_manager_module_deinit() {
	quit_module_deinit();
	leave_module_deinit();
	join_module_deinit();
	room_module_deinit();
	broadcast_module_deinit();
	welcome_module_deinit();
	composite_plugin_destroy(chat_plug);
}


C_CAPSULE_END
