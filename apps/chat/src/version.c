
#include <time.h>
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "streamio.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/version.h"

C_CAPSULE_START

#ifndef GIT_COMMIT_VERSION
#define GIT_COMMIT_VERSION "unknown"
#endif

static int chat_version_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	aroop_txt_t version = {};
	aroop_txt_embeded_set_static_string(&version, "[" GIT_COMMIT_VERSION "]\n");
	chat->strm.send(&chat->strm, &version, 0);
	return 0;
}

static int chat_version_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "version", "chat", plugin_space, __FILE__, "It shows the version and build date.\n");
}

int version_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/version");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_version_plug, chat_version_plug_desc);
	return 0;
}

int version_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_version_plug);
	return 0;
}


C_CAPSULE_END
