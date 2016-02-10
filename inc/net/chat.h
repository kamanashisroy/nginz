#ifndef NGINZ_CHAT_H
#define NGINZ_CHAT_H

C_CAPSULE_START

#define CHAT_SIGNATURE 0x54321

struct chat_connection {
	int fd;
	int (*on_answer)(struct chat_connection*chat, aroop_txt_t*answer); // it is used for prompt
};

NGINZ_INLINE struct composite_plugin*chat_context_get();
int chat_module_init();
int chat_module_deinit();

C_CAPSULE_END

#endif // NGINZ_CHAT_H
