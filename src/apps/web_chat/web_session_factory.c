
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include <sys/socket.h>
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_factory.h"
#include "apps/web_chat/web_chat.h"

C_CAPSULE_START


static struct opp_factory web_session_factory;

static int web_session_factory_on_softquit(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	// iterate through all
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &web_session_factory, OPPN_ALL, 0, 0);
	struct web_session_connection*web_session = NULL;
	while(web_session = opp_iterator_next(&iterator)) {
		web_session->strm.close(&web_session->strm);
	}
	opp_iterator_destroy(&iterator);
	return 0;
}

static int web_session_factory_on_softquit_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "softquitall", "shake", plugin_space, __FILE__, "It tries to destroy all the sessions.\n");
}

static struct web_session_connection*web_session_alloc(int fd, aroop_txt_t*sid) {
	struct web_session_connection*web_session = OPP_ALLOC1(&web_session_factory);
	web_session->strm.fd = fd;
	if(!sid) {
		/* ************************************ */
		/*     Create new session id            */
		/* ************************************ */
		aroop_txt_embeded_buffer(&web_session->sid, NGINZ_MAX_WEBCHAT_SID_SIZE);
		aroop_txt_printf(&web_session->sid, "%d-%d", getpid(), rand());
	} else {
		aroop_txt_embeded_copy_deep(&web_session->sid, sid);
	}
	//syslog(LOG_NOTICE, "saving %d %s\n", aroop_txt_length(&web_session->sid), aroop_txt_to_string(&web_session->sid));
	opp_set_hash(web_session, aroop_txt_get_hash(&web_session->sid)); // set the hash so that it can be searched easily
	struct chat_connection*chat = chat_api_get()->on_create(fd);
	if(!chat) {
		web_session->strm.close(&web_session->strm);
		OPPUNREF(web_session);
		return NULL;
	}
	streamio_chain(&web_session->strm, &chat->strm); // it increases reference count for [chat]
	OPPREF(web_session); // increasing the count do not destroy..
	return web_session;
}

static struct web_session_connection*web_session_search(aroop_txt_t*sid) {
	struct web_session_connection*wchat = NULL;
	//syslog(LOG_NOTICE, "finding %d %s\n", aroop_txt_length(sid), aroop_txt_to_string(sid));
	opp_search(&web_session_factory, aroop_txt_get_hash(sid), NULL, NULL, (void**)&wchat);
	return wchat;
}

static struct web_session_hooks web_hooks = {
	.alloc = web_session_alloc,
	.search = web_session_search,
};

int web_session_close_wrapper(struct streamio*strm) {
	struct web_session_connection*web_session = (struct web_session_connection*)strm;
	web_session->last_activity = 0;
	aroop_txt_embeded_rebuild_and_set_static_string(&web_session->sid, "invalid");
	return default_streamio_close(strm);
}

int web_session_send_wrapper(struct streamio*strm, aroop_txt_t*content, int flags) {
	struct web_session_connection*web_session = (struct web_session_connection*)strm;
	if(aroop_txt_capacity(&web_session->msg) < NGINZ_MAX_WEBCHAT_MSG_SIZE) {
		aroop_txt_embeded_buffer(&web_session->msg, NGINZ_MAX_WEBCHAT_MSG_SIZE); // create cache memory
		if(aroop_txt_capacity(&web_session->msg) < NGINZ_MAX_WEBCHAT_MSG_SIZE)
			return -1;
	}
	if(aroop_txt_is_empty(&web_session->msg)) {
		aroop_txt_concat(&web_session->msg, &web_session->sid);
		aroop_txt_concat_char(&web_session->msg, '\n');
	}
	if(!aroop_txt_is_empty_magical(content)) {
		aroop_txt_concat(&web_session->msg, content);
	}
	if(!aroop_txt_is_empty(&web_session->msg) && !(flags & MSG_MORE) && (web_session->strm.bubble_up)) { // if there is no more data 
		default_streamio_send(&web_session->strm, &web_session->msg, 0); // send the message (through http tunnel may be)
		aroop_txt_set_length(&web_session->msg, 0); // next time we start empty message cache
	}
	return 0;
}

OPP_CB(web_session_connection) {
	struct web_session_connection*web_session = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			aroop_memclean_raw2(&web_session->sid);
			aroop_memclean_raw2(&web_session->msg);
			streamio_initialize(&web_session->strm);
			web_session->strm.send = web_session_send_wrapper;
			web_session->strm.close = web_session_close_wrapper;
			web_session->last_activity = time(NULL);
		break;
		case OPPN_ACTION_FINALIZE:
			streamio_finalize(&web_session->strm);
			aroop_txt_destroy(&web_session->sid);
			aroop_txt_destroy(&web_session->msg);
		break;
	}
	return 0;
}

static int gear = 0;
static int web_session_cleanup_fiber(int status) {
	time_t now = time(NULL);
	if(gear == 1000) {
		gear = 0;
		struct opp_iterator iterator = {};
		opp_iterator_create(&iterator, &web_session_factory, OPPN_ALL, 0, 0);
		struct web_session_connection*web_session = NULL;
		while(web_session = opp_iterator_next(&iterator)) {
			if(now - web_session->last_activity < 120) {
				continue;
			}
			web_session->strm.close(&web_session->strm);
			OPPUNREF(web_session); // cleanup
		}
		opp_iterator_destroy(&iterator);
	}
	gear++;
	return 0;
}

int web_session_factory_module_init() {
	NGINZ_SEARCHABLE_FACTORY_CREATE(&web_session_factory, 64, sizeof(struct web_session_connection), OPP_CB_FUNC(web_session_connection));
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, web_session_factory_on_softquit, web_session_factory_on_softquit_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "web_session/api/hookup");
	composite_plugin_bridge_call(pm_get(), &plugin_space, WEB_CHAT_SIGNATURE, &web_hooks);
	register_fiber(web_session_cleanup_fiber);
}

int web_session_factory_module_deinit() {
	unregister_fiber(web_session_cleanup_fiber);
	pm_unplug_callback(0, web_session_factory_on_softquit);
	web_session_factory_on_softquit(NULL,NULL);
	OPP_PFACTORY_DESTROY(&web_session_factory);
	return 0;
}

C_CAPSULE_END
