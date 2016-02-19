#ifndef NGINZ_CHAT_H
#define NGINZ_CHAT_H

C_CAPSULE_START

#define CHAT_SIGNATURE 0x54321

enum chat_state {
	CHAT_CONNECTED = 0,
	CHAT_LOGGED_IN = 1<<1,
	CHAT_IN_ROOM = 1<<2,
	CHAT_QUIT = 1<<3,
	CHAT_SOFT_QUIT = 1<<4
};

struct chat_hooks {
	struct chat_connection*(*on_create)(int fd);
	//int (*on_client_data)(int fd, int status, const void*cb_data); // it is used to read user input
	int (*on_command)(struct chat_connection*chat, aroop_txt_t*cmd); // it is used for command
	int (*handle_chat_request)(struct chat_connection*chat, aroop_txt_t*request); // it processes chat request
};

struct chat_connection {
	//struct opp_object_ext _ext;
	int fd;
	enum chat_state state;
	aroop_txt_t name;
	aroop_txt_t*request;
	int (*on_response_callback)(struct chat_connection*chat, aroop_txt_t*msg); // it is used for broadcast/setting the name
	const void*callback_data; // XXX do not unref/ref it ..
	int (*send)(struct chat_connection*chat, aroop_txt_t*content, int flag);
	opp_callback_t opp_cb; // it helps to extend chat_connection
};

NGINZ_INLINE struct composite_plugin*chat_context_get();
int chat_module_init();
int chat_module_deinit();

C_CAPSULE_END

#endif // NGINZ_CHAT_H
