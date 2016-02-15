
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
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"

C_CAPSULE_START


static struct chat_hooks hooks = {};
int chat_module_init() {
	memset(&hooks, 0, sizeof(hooks));
	chat_plugin_manager_module_init();

	// setup the hooks
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chatproto/hookup");
	composite_plugin_bridge_call(pm_get(), &plugin_space, CHAT_SIGNATURE, &hooks);
}

int chat_module_deinit() {
	chat_plugin_manager_module_deinit();
}


C_CAPSULE_END
