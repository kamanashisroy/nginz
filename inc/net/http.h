#ifndef NGINZ_HTTP_H
#define NGINZ_HTTP_H

C_CAPSULE_START

#define HTTP_SIGNATURE 0x54332

enum http_state {
	HTTP_CONNECTED = 0,
	HTTP_LOGGED_IN,
	HTTP_IN_ROOM,
	HTTP_QUIT,
	HTTP_SOFT_QUIT
};

struct http_hooks {
	struct http_connection*(*on_create)(int fd);
	int (*on_client_data)(int fd, int status, const void*cb_data); // it is used to read user input
	int (*on_destroy)(struct http_connection**http);
};

struct http_connection {
	//struct opp_object_ext _ext;
	int fd;
	enum http_state state;
	aroop_txt_t*request;
};

NGINZ_INLINE struct composite_plugin*http_context_get();
int http_module_init();
int http_module_deinit();

C_CAPSULE_END

#endif // NGINZ_HTTP_H
