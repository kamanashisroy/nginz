
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "inc/plugin.h"
#include "inc/plugin_manager.h"
#include "inc/fiber.h"

C_CAPSULE_START



static struct composite_plugin*coreplug = NULL;
int pm_call(aroop_txt_t*plugin_space, aroop_txt_t*input, aroop_txt_t*output) {
	aroop_assert(coreplug != NULL);
	composite_plugin_call(coreplug, plugin_space, input, output);
}

struct composite_plugin*pm_get() {
	return coreplug;
}

static int plugin_command_helper(
	int category
	, aroop_txt_t*plugin_space
	, int(*callback)(aroop_txt_t*input, aroop_txt_t*output)
	, int(*bridge)(int signature, void*x)
	, struct composite_plugin*inner
	, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)
	, void*visitor_data
) {
	aroop_txt_t*output = (aroop_txt_t*)visitor_data;
	aroop_txt_t xdesc = {};
	desc(plugin_space, &xdesc);
	aroop_txt_concat(output, &xdesc);
	aroop_txt_destroy(&xdesc);
}

static int plugin_command(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 1024);
	composite_plugin_visit_all(pm_get(), plugin_command_helper, output);
	return 0;
}

static int plugin_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "plugin", "shake", plugin_space, __FILE__, "It dumps the avilable plugins\n");
}

int pm_init() {
	plugin_init();
	aroop_assert(coreplug == NULL);
	coreplug = composite_plugin_create();
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/plugin");
	pm_plug_callback(&plugin_space, plugin_command, plugin_command_desc);
}

int pm_deinit() {
	composite_plugin_destroy(coreplug);
	plugin_deinit();
}

C_CAPSULE_END
