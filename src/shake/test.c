
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "shake.h"

C_CAPSULE_START

static int test_command_tester(
	int category
	, aroop_txt_t*plugin_space
	, int(*callback)(aroop_txt_t*input, aroop_txt_t*output)
	, int(*bridge)(int signature, void*x)
	, struct composite_plugin*inner
	, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)
	, void*visitor_data
) {
	aroop_txt_t prefix = {};
	aroop_txt_t input = {};
	aroop_txt_t*output = (aroop_txt_t*)visitor_data;
	aroop_txt_t xdesc = {};
	aroop_txt_embeded_copy_on_demand(&prefix, plugin_space);
	aroop_txt_set_length(&prefix, 5);
	if(!aroop_txt_equals_static(&prefix, "test/"))
		return 0;
	callback(&input, &xdesc);
	aroop_txt_concat(output, &xdesc);
	aroop_txt_destroy(&xdesc);
}

static int test_command(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 512);
	composite_plugin_visit_all(pm_get(), test_command_tester, output);
	printf("test complete\n");
}

static int test_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "test", "shake", plugin_space, __FILE__, "It tests all the test cases.\n");
}

int test_module_init() {
	aroop_txt_t test_plug;
	aroop_txt_embeded_set_static_string(&test_plug, "shake/test");
	pm_plug_callback(&test_plug, test_command, test_command_desc);
}

int test_module_deinit() {
	pm_unplug_callback(0, test_command);
}

C_CAPSULE_END
