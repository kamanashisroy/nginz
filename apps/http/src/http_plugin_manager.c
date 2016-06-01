
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_str2.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "protostack.h"
#include "streamio.h"
#include "http.h"
#include "http/http_tunnel.h"
#include "http/http_plugin_manager.h"

C_CAPSULE_START

static struct composite_plugin*http_plug = NULL;

NGINZ_INLINE struct composite_plugin*http_plugin_manager_get() {
	return http_plug;
}

static int http_plugin_command(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 1024);
	composite_plugin_visit_all(http_plugin_manager_get(), plugin_command_helper, output);
	return 0;
}

static int http_plugin_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "httpplugin", "shake", plugin_space, __FILE__, "It dumps the avilable plugins\n");
}

static int http_help_plug_helper(
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
		if(!aroop_txt_equals_static(&prefix, "http/"))
			break;
		if(aroop_txt_char_at(&prefix, 5) == '_') // hide the hidden commands
			break;
		desc(plugin_space, &xdesc);
		aroop_txt_concat(output, &xdesc);
	} while(0);
	aroop_txt_destroy(&xdesc); // cleanup
	aroop_txt_destroy(&prefix); // cleanup
	return 0;
}

static int http_help_plug(int signature, void*given) {
	aroop_assert(signature == HTTP_SIGNATURE);
	struct http_connection*http = (struct http_connection*)given;
	if(!IS_VALID_HTTP(http)) // sanity check
		return 0;
	if(!http_plugin_manager_get()) { // sanity check
		return 0;
	}
	aroop_txt_t output = {};
	aroop_txt_embeded_stackbuffer(&output, 1024);
	composite_plugin_visit_all(http_plugin_manager_get(), http_help_plug_helper, &output);
	http->strm.send(&http->strm, &output, 0);
	return 0;
}

static int http_help_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "help", "http", plugin_space, __FILE__, "It shows available commands.\n");
}

int http_plugin_manager_module_init() {
	aroop_assert(http_plug == NULL);
	http_plug = composite_plugin_create();
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "http/help");
	cplug_bridge(http_plugin_manager_get(), &plugin_space, http_help_plug, http_help_plug_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/httpplugin");
	pm_plug_callback(&plugin_space, http_plugin_command, http_plugin_command_desc);
	http_tunnel_module_init();
	return 0;
}

int http_plugin_manager_module_deinit() {
	http_tunnel_module_deinit();
	composite_unplug_bridge(http_plugin_manager_get(), 0, http_help_plug);
	composite_plugin_destroy(http_plug);
	return 0;
}


C_CAPSULE_END
