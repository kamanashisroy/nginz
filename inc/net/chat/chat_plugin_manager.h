#ifndef NGINZ_CHAT_PLUGIN_MANAGER_H
#define NGINZ_CHAT_PLUGIN_MANAGER_H

C_CAPSULE_START

NGINZ_INLINE struct composite_plugin*chat_plugin_manager_get();
int chat_plugin_manager_module_init();
int chat_plugin_manager_module_deinit();

C_CAPSULE_END

#endif // NGINZ_CHAT_PLUGIN_MANAGER_H
