#ifndef NGINZ_CHAT_H
#define NGINZ_CHAT_H

C_CAPSULE_START

#define CHAT_SIGNATURE 0x54321

enum chat_state {
	CHAT_CONNECTED = 0,
	CHAT_LOGGED_IN,
	CHAT_IN_ROOM,
	CHAT_QUIT,
	CHAT_SOFT_QUIT
};

struct chat_connection {
	//struct opp_object_ext _ext;
	int fd;
	enum chat_state state;
	aroop_txt_t name;
	aroop_txt_t*request;
	int (*on_answer)(struct chat_connection*chat, aroop_txt_t*answer); // it is used for prompt
	const void*answer_data; // XXX do not unref/ref it ..
	int (*on_broadcast)(struct chat_connection*chat, aroop_txt_t*msg); // it is used for prompt
	const void*broadcast_data; // XXX do not unref/ref it ..
};

NGINZ_INLINE struct composite_plugin*chat_context_get();
int chat_module_init();
int chat_module_deinit();

C_CAPSULE_END

#endif // NGINZ_CHAT_H
