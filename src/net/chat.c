
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"

C_CAPSULE_START

static struct chat_api hooks = {};
#if 0
static int chat_rehash_plug(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chatproto/hookup");
	composite_plugin_bridge_call(pm_get(), &plugin_space, CHAT_SIGNATURE, &hooks);
	return 0;
}

static int chat_rehash_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "chat_module", "chat hooking", plugin_space, __FILE__, "It rehashes the setup.\n");
}
#endif

struct chat_api*chat_api_get() {
	return &hooks;
}

int chat_module_init() {
	memset(&hooks, 0, sizeof(hooks));
	chat_plugin_manager_module_init();

#if 0
	// setup the hooks
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chatproto/hookup");
	composite_plugin_bridge_call(pm_get(), &plugin_space, CHAT_SIGNATURE, &hooks);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/rehash");
	pm_plug_callback(&plugin_space, chat_rehash_plug, chat_rehash_plug_desc);
#endif
}

int chat_module_deinit() {
	chat_plugin_manager_module_deinit();
#if 0
	pm_unplug_callback(0, chat_rehash_plug);
#endif
}


C_CAPSULE_END
