
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginez_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/chat.h"

C_CAPSULE_START

static struct composite_plugin*chat_plug = NULL;

NGINEZ_INLINE struct composite_plugin*chat_context_get() {
	return chat_plug;
}

#if 0
NGINEZ_INLINE int chat_interface_init(struct chat_interface*x, int fd) {
	x->fd = fd;
	memset(&x->input, 0, sizeof(x->input));
}

NGINEZ_INLINE int chat_interface_destroy(struct chat_interface*x) {
	aroop_txt_destroy(&x->input);
}
#endif

static aroop_txt_t chat_welcome = {};
static int chat_on_connect(int fd) {
	composite_plugin_bridge_call(chat_context_get(), &chat_welcome, CHAT_SIGNATURE, &fd);
	return 0;
}

struct protostack chat_protostack = {
	.on_connect = chat_on_connect,
};

int chat_module_init() {
	aroop_txt_embeded_set_static_string(&chat_welcome, "chat/welcome");
	aroop_assert(chat_plug == NULL);
	chat_plug = composite_plugin_create();
	protostack_set(NGINEZ_DEFAULT_PORT, &chat_protostack);
	welcome_module_init();
}

int chat_module_deinit() {
	welcome_module_deinit();
	composite_plugin_destroy(chat_plug);
}


C_CAPSULE_END
