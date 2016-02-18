#ifndef NGINZ_HTTP_HTTP_PLUGIN_MANAGER_H
#define NGINZ_HTTP_HTTP_PLUGIN_MANAGER_H

C_CAPSULE_START

int http_tunnel_send_content(struct http_connection*http, aroop_txt_t*content);
int http_plugin_manager_module_init();
int http_plugin_manager_module_deinit();

C_CAPSULE_END

#endif // NGINZ_HTTP_HTTP_PLUGIN_MANAGER_H
