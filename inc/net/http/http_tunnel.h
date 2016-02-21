#ifndef NGINZ_HTTP_TUNNEL_H
#define NGINZ_HTTP_TUNNEL_H

C_CAPSULE_START

int http_tunnel_send_content(struct streamio*strm, aroop_txt_t*content, int flags);
int http_tunnel_module_init();
int http_tunnel_module_deinit();

C_CAPSULE_END

#endif // NGINZ_HTTP_TUNNEL_H
