
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
#include "protostack.h"
#include "streamio.h"
#include "chat.h"
#include "chat_subsystem.h"
#include "chat/chat_plugin_manager.h"

C_CAPSULE_START

static struct chat_api hooks = {};

struct chat_api*chat_api_get() {
	return &hooks;
}

int nginz_chat_module_init() {
	memset(&hooks, 0, sizeof(hooks));
	chat_plugin_manager_module_init();
	return 0;
}

int nginz_chat_module_deinit() {
	chat_plugin_manager_module_deinit();
	return 0;
}

C_CAPSULE_END
