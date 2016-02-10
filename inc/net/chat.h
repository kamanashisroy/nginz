#ifndef NGINZ_CHAT_H
#define NGINZ_CHAT_H

C_CAPSULE_START

#define CHAT_SIGNATURE 0x54321

enum chat_state {
	CHAT_CONNECTED = 0,
	CHAT_LOGGED_IN,
	CHAT_IN_ROOM,
	CHAT_QUIT
};

struct chat_connection {
	int fd;
	enum chat_state state;
	int (*on_answer)(struct chat_connection*chat, aroop_txt_t*answer); // it is used for prompt
};

NGINZ_INLINE struct composite_plugin*chat_context_get();
int chat_module_init();
int chat_module_deinit();

C_CAPSULE_END

#endif // NGINZ_CHAT_H
