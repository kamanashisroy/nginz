
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "log.h"
#include <signal.h>
#include "plugin_manager.h"
#include "fiber.h"
#include "fork.h"
#include "db.h"
#include "shake/quitall.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "event_loop.h"
#include "apps/web_chat/web_chat.h"

C_CAPSULE_START

static int rehash() {
	aroop_txt_t input = {};
	aroop_txt_t output = {};
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/rehash");
	pm_call(&plugin_space, &input, &output);
	aroop_txt_destroy(&input);
	aroop_txt_destroy(&output);
	return 0;
}

static void signal_callback(int sigval) {
	fiber_quit();
}

static int nginz_master_init() {
	/**
	 * Setup for master
	 */
	shake_module_init(); // we start the control fd after fork 
	tcp_listener_init();
	return 0;
}

static int nginz_init() {
#ifdef HAS_MEMCACHED_MODULE
	db_module_init();
#endif
	pm_init();
	pp_module_init();
	shake_quitall_module_init();
	binary_coder_module_init();
	fiber_module_init();
	event_loop_module_init();
	protostack_init();
#ifdef HAS_CHAT_MODULE
	chat_module_init();
#endif
#ifdef HAS_HTTP_MODULE
	http_module_init();
#endif
#ifdef HAS_WEB_CHAT_MODULE
	web_chat_module_init();
#endif
	rehash();
	signal(SIGPIPE, SIG_IGN); // avoid crash on sigpipe
	signal(SIGINT, signal_callback);
	fork_processors(NGINZ_NUMBER_OF_PROCESSORS);
	nginz_master_init();
	return 0;
}

static int nginz_deinit() {
	tcp_listener_deinit();
#ifdef HAS_WEB_CHAT_MODULE
	web_chat_module_deinit();
#endif
#ifdef HAS_HTTP_MODULE
	http_module_deinit();
#endif
#ifdef HAS_CHAT_MODULE
	chat_module_deinit();
#endif
	protostack_deinit();
	event_loop_module_deinit();
	shake_module_deinit();
	fiber_module_deinit();
	binary_coder_module_deinit();
	shake_quitall_module_deinit();
	pp_module_deinit();
	pm_deinit();
#ifdef HAS_MEMCACHED_MODULE
	db_module_deinit();
#endif
}

static int nginz_main(char**args) {
	daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	nginz_init();
	fiber_module_run();
	nginz_deinit();
	closelog();
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, nginz_main);
}

C_CAPSULE_END
