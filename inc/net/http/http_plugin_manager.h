#ifndef NGINZ_HTTP_HTTP_PLUGIN_MANAGER_H
#define NGINZ_HTTP_HTTP_PLUGIN_MANAGER_H

C_CAPSULE_START

NGINZ_INLINE struct composite_plugin*http_plugin_manager_get();
int http_plugin_manager_module_init();
int http_plugin_manager_module_deinit();

C_CAPSULE_END

#endif // NGINZ_HTTP_HTTP_PLUGIN_MANAGER_H
