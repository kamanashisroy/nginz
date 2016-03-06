
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_hash_table.h>
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "parallel/pipeline.h"
#include "parallel/async_request.h"

C_CAPSULE_START

//#define DB_LOG(...) syslog(__VA_ARGS__)
#define DB_LOG(...)

static aroop_txt_t null_hook = {};
int async_db_compare_and_swap(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key, aroop_txt_t*newval, aroop_txt_t*oldval) {
	if(cb_hook == NULL) {
		cb_hook = &null_hook;
	}
	aroop_assert(!aroop_txt_is_empty_magical(key));
	if(oldval)
		aroop_assert(!aroop_txt_is_empty_magical(newval));
	aroop_txt_t app = {};
	if(oldval) {
		aroop_txt_embeded_set_static_string(&app, "asyncdb/cas/request"); 
	} else {
		if(newval)
			aroop_txt_embeded_set_static_string(&app, "asyncdb/sin/request"); 
		else
			aroop_txt_embeded_set_static_string(&app, "asyncdb/unset/request"); 
	}
	aroop_txt_t*args[4] = {key, newval, oldval, NULL};
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = key, 6 = newval, 7 = oldval
	DB_LOG(LOG_NOTICE, "[token%d]-CAS-throwing to--[master]-[key:%s]", cb_token, aroop_txt_to_string(key));
	async_pm_call_master(cb_token, cb_hook, &app, args);
	return 0;
}

int async_db_unset(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key) {
	if(cb_hook == NULL) {
		cb_hook = &null_hook;
	}
	return async_db_compare_and_swap(cb_token, cb_hook, key, NULL, NULL);
}

int async_db_set_int(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key, int intval) {
	if(cb_hook == NULL) {
		cb_hook = &null_hook;
	}
	aroop_txt_t intstr = {};
	aroop_txt_embeded_stackbuffer(&intstr, 32);
	aroop_txt_printf(&intstr, "%d", intval);
	async_db_compare_and_swap(cb_token, cb_hook, key, &intstr, NULL);
	aroop_txt_destroy(&intstr);
}

int async_db_get(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key) {
	if(cb_hook == NULL) {
		cb_hook = &null_hook;
	}
	aroop_assert(!aroop_txt_is_empty_magical(key));
	aroop_txt_t app = {};
	aroop_txt_embeded_set_static_string(&app, "asyncdb/get/request"); 
	aroop_txt_t*args[2] = {key, NULL};
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = key
	DB_LOG(LOG_NOTICE, "[token%d]-get-throwing to--[master]-[key:%s]", cb_token, aroop_txt_to_string(key));
	return async_pm_call_master(cb_token, cb_hook, &app, args);
}

int async_db_init() {
	aroop_txt_embeded_set_static_string(&null_hook, "null");
	if(is_master()) {
		async_db_master_init();
	}
	return 0;
}

int async_db_deinit() {
	if(is_master())
		async_db_master_deinit();
}

C_CAPSULE_END

