
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "plugin.h"
#include "plugin_manager.h"
#include "net/chat.h"

C_CAPSULE_START

int chat_module_init() {
	welcome_module_init();
}

int chat_module_deinit() {
	welcome_module_deinit();
}


C_CAPSULE_END
