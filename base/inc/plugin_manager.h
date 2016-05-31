#ifndef NGINZ_PLUGIN_MANAGER_H
#define NGINZ_PLUGIN_MANAGER_H

C_CAPSULE_START


struct composite_plugin*composite_plugin_create();
#define composite_plugin_destroy OPPUNREF
int pm_call(aroop_txt_t*plugin_space, aroop_txt_t*input, aroop_txt_t*output);
int pm_bridge_call(aroop_txt_t*plugin_space, int signature, void*data);

struct composite_plugin*pm_get();
int plugin_command_helper(
	int category
	, aroop_txt_t*plugin_space
	, int(*callback)(aroop_txt_t*input, aroop_txt_t*output)
	, int(*bridge)(int signature, void*x)
	, struct composite_plugin*inner
	, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)
	, void*visitor_data
);

#define pm_plug_callback(plugin_space, callback, desc) ({cplug_callback(pm_get(), plugin_space, callback, desc);})
#define pm_unplug_callback(plugin_id, callback) ({composite_unplug_callback(pm_get(), plugin_id, callback);})

#define pm_plug_bridge(target, bridge, desc) ({cplug_bridge(pm_get(), target, bridge, desc);})
#define pm_unplug_bridge(plugin_id, bridge) ({composite_unplug_bridge(pm_get(), plugin_id, bridge);})

int pm_init();
int pm_deinit();


C_CAPSULE_END

#endif // NGINZ_PLUGIN_MANAGER_H
