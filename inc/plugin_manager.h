#ifndef NGINZ_PLUGIN_MANAGER_H
#define NGINZ_PLUGIN_MANAGER_H

C_CAPSULE_START


struct composite_plugin*composite_plugin_create();
#define composite_plugin_destroy OPPUNREF
int pm_call(aroop_txt_t*plugin_space, aroop_txt_t*input, aroop_txt_t*output);
struct composite_plugin*pm_get();

#define pm_plug_callback(plugin_space, callback, desc) ({composite_plug_callback(pm_get(), plugin_space, callback, desc);})
#define pm_unplug_callback(plugin_id, callback) ({composite_unplug_callback(pm_get(), plugin_id, callback);})

int pm_init();
int pm_deinit();


C_CAPSULE_END

#endif // NGINZ_PLUGIN_MANAGER_H
