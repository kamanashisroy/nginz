#ifndef NGINEZ_CHAT_H
#define NGINEZ_CHAT_H

C_CAPSULE_START

struct chat_interface {
	int fd;
	aroop_txt_t input;
};

#define CHAT_SIGNATURE 0x54321

NGINEZ_INLINE int chat_interface_init(struct chat_interface*x, int fd);
NGINEZ_INLINE int chat_interface_destroy(struct chat_interface*x);
struct composite_plugin*chat_context_get();
int chat_module_init();
int chat_module_deinit();

C_CAPSULE_END

#endif // NGINEZ_CHAT_H
