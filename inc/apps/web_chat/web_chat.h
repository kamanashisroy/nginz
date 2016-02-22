#ifndef NGINZ_WEB_CHAT_H
#define NGINZ_WEB_CHAT_H

C_CAPSULE_START

enum {
	WEB_CHAT_SIGNATURE = 12923,
};

struct web_session_connection {
	struct streamio strm;
	aroop_txt_t sid;
	aroop_txt_t msg;
};

struct web_session_hooks {
	struct web_session_connection*(*alloc)(int fd, aroop_txt_t*sid); // factory pattern
	struct web_session_connection*(*search)(aroop_txt_t*sid);
};

int web_chat_module_init();
int web_chat_module_deinit();

C_CAPSULE_END

#endif // NGINZ_WEB_CHAT_H
