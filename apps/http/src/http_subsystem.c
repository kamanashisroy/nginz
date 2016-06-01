
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
#include "http.h"
#include "http_subsystem.h"
#include "http/http_factory.h"
#include "http/http_accept.h"
#include "http/http_parser.h"
#include "http/http_plugin_manager.h"

C_CAPSULE_START

static struct http_hooks hooks = {};
int nginz_http_module_init() {
	memset(&hooks, 0, sizeof(hooks));
	http_factory_module_init();
	http_accept_module_init();
	http_parser_module_init();
	http_plugin_manager_module_init();

	// setup the hooks
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "httpproto/hookup");
	composite_plugin_bridge_call(pm_get(), &plugin_space, HTTP_SIGNATURE, &hooks);
	return 0;
}

int nginz_http_module_deinit() {
	http_plugin_manager_module_deinit();
	http_parser_module_deinit();
	http_accept_module_deinit();
	http_factory_module_deinit();
	return 0;
}


C_CAPSULE_END
