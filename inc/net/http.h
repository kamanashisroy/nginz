#ifndef NGINZ_HTTP_H
#define NGINZ_HTTP_H

C_CAPSULE_START

#define HTTP_SIGNATURE 0x54332

enum http_state {
	HTTP_CONNECTED = 0,
	HTTP_QUIT,
	HTTP_SOFT_QUIT,
	HTTP_RESERVED1,
	HTTP_RESERVED2,
	HTTP_RESERVED3,
	HTTP_RESERVED4,
};

struct http_hooks {
	struct http_connection*(*on_create)(int fd); // factory pattern
	int (*on_client_data)(int fd, int status, const void*cb_data); // it is used to read user input from event_loop
};

struct http_connection {
	//struct opp_object_ext _ext;
	int fd;
	enum http_state state;
	aroop_txt_t*request;
	int is_processed;
	opp_callback_t opp_cb; // it helps to extend http_connection
};

NGINZ_INLINE struct composite_plugin*http_context_get();
int http_module_init();
int http_module_deinit();

C_CAPSULE_END

#endif // NGINZ_HTTP_H
