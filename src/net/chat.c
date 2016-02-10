
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginez_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/chat.h"

C_CAPSULE_START

static struct composite_plugin*chat_plug = NULL;

struct composite_plugin*chat_context_get() {
	return chat_plug;
}

NGINEZ_INLINE int chat_interface_init(struct chat_interface*x, int fd) {
	x->fd = fd;
	memset(&x->input, 0, sizeof(x->input));
}

NGINEZ_INLINE int chat_interface_destroy(struct chat_interface*x) {
	aroop_txt_destroy(&x->input);
}

int chat_module_init() {
	aroop_assert(chat_plug == NULL);
	chat_plug = composite_plugin_create();
	welcome_module_init();
}

int chat_module_deinit() {
	welcome_module_deinit();
	composite_plugin_destroy(chat_plug);
}


C_CAPSULE_END
