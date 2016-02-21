#ifndef NGINZ_HTTP_H
#define NGINZ_HTTP_H

C_CAPSULE_START

#define HTTP_SIGNATURE 0x54332

enum http_state {
	HTTP_CONNECTED = 0,
	HTTP_QUIT = 1<<3,
	HTTP_SOFT_QUIT = 1<<4
};

struct http_hooks {
	struct http_connection*(*on_create)(int fd); // factory pattern
	int (*on_client_data)(int fd, int status, const void*cb_data); // it is used to read user input from event_loop
};

struct http_connection {
	struct streamio strm;
	enum http_state state;
	int is_processed;
	aroop_txt_t content;
};

int http_module_init();
int http_module_deinit();

#define IS_VALID_HTTP(x) (NULL != (x) && IS_VALID_STREAM(&x->strm))

C_CAPSULE_END

#endif // NGINZ_HTTP_H
