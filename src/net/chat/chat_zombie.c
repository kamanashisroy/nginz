
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include <aroop/opp/opp_list.h>
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "log.h"
#include "fiber.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/chat_zombie.h"

C_CAPSULE_START

static struct opp_factory zombie_list;


int chat_zombie_add(struct chat_connection*chat) {
	if(chat->state & CHAT_ZOMBIE) // it is already added
		return -1;
	aroop_assert(chat->quited_at);
	chat->state |= CHAT_ZOMBIE;
	opp_list_add_noref(&zombie_list, chat);
	OPPREF(chat);
	return 0;
}

static int chat_zombie_on_softquit(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	struct opp_iterator iterator;
	opp_iterator_create(&iterator, &zombie_list, OPPN_ALL, 0, 0);
	do {
		opp_pointer_ext_t*pt = opp_iterator_next(&iterator);
		if(pt == NULL)
			break;
		OPPUNREF(pt);
	} while(1);
	opp_iterator_destroy(&iterator);
	return 0;
}

static int chat_zombie_on_softquit_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "softquitall", "shake", plugin_space, __FILE__, "It tries to destroy all the zombie connections.\n");
}

static long gear = 0;
static int chat_lazy_cleanup_fiber(int status) {
	if(gear < 1000000) {
		gear++;
		return 0;
	}
	time_t now = time(NULL);
	gear = 0;
	if(!OPP_FACTORY_USE_COUNT(&zombie_list))
		return 0;
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &zombie_list, OPPN_ALL, 0, 0);
	do {
		opp_pointer_ext_t*pt = NULL;
		pt = opp_iterator_next(&iterator);
		if(!pt)
			break;
		struct chat_connection*chat = (struct chat_connection*)pt->obj_data;
		if(chat->quited_at == 0 || ((now - chat->quited_at) < 120)) {
			continue;
		}
		OPPUNREF(pt); // cleanup
	}while(1);
	opp_iterator_destroy(&iterator);
	return 0;
}


int chat_zombie_module_init() {
	OPP_LIST_CREATE_NOLOCK(&zombie_list, 64);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, chat_zombie_on_softquit, chat_zombie_on_softquit_desc);
	register_fiber(chat_lazy_cleanup_fiber);
	return 0;
}

int chat_zombie_module_deinit() {
	unregister_fiber(chat_lazy_cleanup_fiber);
	pm_unplug_callback(0, chat_zombie_on_softquit);
	OPP_PFACTORY_DESTROY(&zombie_list);
	return 0;
}

C_CAPSULE_END
