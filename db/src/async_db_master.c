
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_hash_table.h>
#include <aroop/opp/opp_str2.h>
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "parallel/pipeline.h"
#include "parallel/async_request.h"
#include "binary_coder.h"
#include "async_db.h"
#include "async_db_master.h"
#include "async_db_internal.h"

C_CAPSULE_START

//#define DB_LOG(...) syslog(__VA_ARGS__)
#define DB_LOG(...)

static opp_hash_table_t global_db; // TODO create partitioned db in future
int noasync_db_get(aroop_txt_t*key, aroop_txt_t*val) {
	aroop_assert(is_master());
	aroop_txt_t*oldval = (aroop_txt_t*)opp_hash_table_get_no_ref(&global_db, key); // no cleanup needed
	if(oldval)
		aroop_txt_embeded_rebuild_copy_shallow(val, oldval); // needs cleanup
	return 0;
}

static aroop_txt_t null_hook = {};
static int async_db_op_reply(int destpid, int cb_token, aroop_txt_t*cb_hook, int success, aroop_txt_t*key, aroop_txt_t*newval) {
	// send response
	// 0 = pid, 1 = src pid, 2 = command, 3 = token, 4 = cb_hook, 5 = success
	aroop_txt_t*args[3] = {key, newval, NULL};
	DB_LOG(LOG_NOTICE, "[token%d]-replying-throwing to--[dest:%d]-[key:%s]-[app:%s]", cb_token, destpid, aroop_txt_to_string(key), aroop_txt_to_string(cb_hook));
	if(aroop_txt_is_empty(cb_hook)) {
		return async_pm_reply_worker(destpid, cb_token, &null_hook, success, args);
	}
	return async_pm_reply_worker(destpid, cb_token, cb_hook, success, args);
}

static int async_db_op_helper(aroop_txt_t*key, aroop_txt_t*newval, aroop_txt_t*expval) {
	if(aroop_txt_is_empty(key)) { // we cannot process the request ..
		return -1;
	}
	if(newval == NULL) {
		opp_hash_table_set(&global_db, key, NULL); // unset
		return 0;
	}
	aroop_txt_t*oldval = (aroop_txt_t*)opp_hash_table_get_no_ref(&global_db, key); // no cleanup needed
	if(!((oldval == NULL && expval == NULL) || (oldval != NULL && expval != NULL && aroop_txt_equals(expval, oldval)))) {
		return -1;
	}
	aroop_txt_t*xnval = aroop_txt_new_copy_deep(newval, NULL);
	aroop_txt_t*xkey = aroop_txt_new_copy_deep(key, NULL);
	DB_LOG(LOG_NOTICE, "--op----[key:%s]", aroop_txt_to_string(key));
	opp_hash_table_set(&global_db, xkey, xnval);
	OPPUNREF(xnval);
	OPPUNREF(xkey);
	return 0;
}

static int async_db_CAS_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	/**
	 * The database is available only in the master process
	 */
	aroop_assert(is_master());
	aroop_txt_t key = {}; // the key to set
	aroop_txt_t expval = {}; // the val to compare with the oldval
	aroop_txt_t newval = {}; // the val to set
	int srcpid = 0;
	int cb_token = 0;
	aroop_txt_t cb_hook = {};
	// 0 = srcpid, 1 = command, 2 = token, 3 = cb_hook, 4 = key, 5 = newval, 6 = oldval
	binary_unpack_int(bin, 0, &srcpid);
	binary_unpack_int(bin, 2, &cb_token);
	binary_unpack_string(bin, 3, &cb_hook); // needs cleanup
	binary_unpack_string(bin, 4, &key); // needs cleanup
	binary_unpack_string(bin, 5, &newval); // needs cleanup
	binary_unpack_string(bin, 6, &expval); // needs cleanup
	DB_LOG(LOG_NOTICE, "[token%d]-CAS-doing ..--[dest:%d]-[key:%s]-[app:%s]", cb_token, srcpid, aroop_txt_to_string(&key), aroop_txt_to_string(&cb_hook));
	int success = 0;
	success = !async_db_op_helper(&key, &newval, &expval);
#if 0
	if(destpid > 0) {
#endif
		async_db_op_reply(srcpid, cb_token, &cb_hook, success, &key, &newval);
#if 0
	}
#endif
	// cleanup
	aroop_txt_destroy(&key);
	aroop_txt_destroy(&newval);
	aroop_txt_destroy(&expval);
	aroop_txt_destroy(&cb_hook);
	return 0;
}

static int async_db_set_if_null_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	/**
	 * The database is available only in the master process
	 */
	aroop_assert(is_master());
	aroop_txt_t key = {}; // the key to set
	aroop_txt_t newval = {}; // the val to set
	int srcpid = 0;
	int cb_token = 0;
	aroop_txt_t cb_hook = {};
	// 0 = srcpid, 1 = command, 2 = token, 3 = cb_hook, 4 = key, 5 = newval, 6 = oldval
	binary_unpack_int(bin, 0, &srcpid);
	binary_unpack_int(bin, 2, &cb_token);
	binary_unpack_string(bin, 3, &cb_hook); // needs cleanup
	binary_unpack_string(bin, 4, &key); // needs cleanup
	binary_unpack_string(bin, 5, &newval); // needs cleanup
	DB_LOG(LOG_NOTICE, "[token%d]-set if null-doing ..--[dest:%d]-[key:%s]-[app:%s]", cb_token, srcpid, aroop_txt_to_string(&key), aroop_txt_to_string(&cb_hook));
	int success = 0;
	success = !async_db_op_helper(&key, &newval, NULL);
#if 0
	if(destpid > 0) {
#endif
		async_db_op_reply(srcpid, cb_token, &cb_hook, success, &key, &newval);
#if 0
	}
#endif
	// cleanup
	aroop_txt_destroy(&cb_hook);
	aroop_txt_destroy(&key);
	aroop_txt_destroy(&newval);
	return 0;
}

static int async_db_unset_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	/**
	 * The database is available only in the master process
	 */
	aroop_assert(is_master());
	aroop_txt_t key = {}; // the key to set
	int srcpid = 0;
	int cb_token = 0;
	aroop_txt_t cb_hook = {};
	// 0 = srcpid, 1 = command, 2 = token, 3 = cb_hook, 4 = key, 5 = newval, 6 = oldval
	binary_unpack_int(bin, 0, &srcpid);
	binary_unpack_int(bin, 2, &cb_token);
	binary_unpack_string(bin, 3, &cb_hook); // needs cleanup
	binary_unpack_string(bin, 4, &key); // needs cleanup
	DB_LOG(LOG_NOTICE, "[token%d]-unset-doing ..--[dest:%d]-[key:%s]-[app:%s]", cb_token, srcpid, aroop_txt_to_string(&key), aroop_txt_to_string(&cb_hook));
	int success = 0;
	success = !async_db_op_helper(&key, NULL, NULL);
#if 0
	if(destpid > 0) {
#endif
		async_db_op_reply(srcpid, cb_token, &cb_hook, success, &key, NULL);
#if 0
	}
#endif
	// cleanup
	aroop_txt_destroy(&cb_hook);
	aroop_txt_destroy(&key);
	return 0;
}

static int async_db_get_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	/**
	 * The database is available only in the master process
	 */
	aroop_assert(is_master());
	aroop_txt_t key = {}; // the key to set
	int srcpid = 0;
	int cb_token = 0;
	aroop_txt_t cb_hook = {};
	// 0 = srcpid, 1 = command, 2 = token, 3 = cb_hook, 4 = key, 5 = val
	binary_unpack_int(bin, 0, &srcpid);
	binary_unpack_int(bin, 2, &cb_token);
	binary_unpack_string(bin, 3, &cb_hook); // needs cleanup
	binary_unpack_string(bin, 4, &key); // needs cleanup
	DB_LOG(LOG_NOTICE, "[token%d]-get-doing ..--[dest:%d]-[key:%s]-[app:%s]", cb_token, srcpid, aroop_txt_to_string(&key), aroop_txt_to_string(&cb_hook));
	//syslog(LOG_NOTICE, "[pid:%d]-getting:%s", getpid(), aroop_txt_to_string(&key));
	aroop_txt_t*oldval = NULL;
	do {
		oldval = (aroop_txt_t*)opp_hash_table_get_no_ref(&global_db, &key); // no cleanup needed
#if 0
		if(destpid <= 0) {
			break;
		}
#endif
		//syslog(LOG_NOTICE, "[pid:%d]-got:%s", getpid(), aroop_txt_to_string(oldval));
		async_db_op_reply(srcpid, cb_token, &cb_hook, 1, &key, oldval);
	} while(0);
	
	// cleanup
	aroop_txt_destroy(&cb_hook);
	aroop_txt_destroy(&key);
	return 0;
}

static int async_db_hook_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "async db", "asyncdb", plugin_space, __FILE__, "It helps cas and other rpc db values in master process.\n");
}

static int async_db_dump_helper(void*cb_data, void*key, void*value) {
	aroop_txt_t*output = (aroop_txt_t*)cb_data;
	aroop_txt_t*xkey = (aroop_txt_t*)key;
	aroop_txt_t*xval = (aroop_txt_t*)value;
	aroop_txt_concat_char(output, '[');
	aroop_txt_concat(output, xkey);
	aroop_txt_concat_char(output, ']');
	aroop_txt_concat_char(output, '>');
	aroop_txt_concat_char(output, '[');
	aroop_txt_concat(output, xval);
	aroop_txt_concat_char(output, ']');
	aroop_txt_concat_char(output, '\n');
	return 0;
}

static int async_db_dump_hook(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 2048);
	opp_hash_table_traverse(&global_db, async_db_dump_helper, output, OPPN_ALL, 0, 0);
	return 0;
}

static int async_db_dump_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "dbdump", "shake", plugin_space, __FILE__, "It dumps the db entries from database.\n");
}

int async_db_master_init() {
	aroop_txt_embeded_set_static_string(&null_hook, "null");
	aroop_assert(is_master());
	opp_hash_table_create(&global_db, 16, 0, aroop_txt_get_hash_cb, aroop_txt_equals_cb);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "asyncdb/cas/request");
	pm_plug_callback(&plugin_space, async_db_CAS_hook, async_db_hook_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "asyncdb/sin/request");
	pm_plug_callback(&plugin_space, async_db_set_if_null_hook, async_db_hook_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "asyncdb/unset/request");
	pm_plug_callback(&plugin_space, async_db_unset_hook, async_db_hook_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "asyncdb/get/request");
	pm_plug_callback(&plugin_space, async_db_get_hook, async_db_hook_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/dbdump");
	pm_plug_callback(&plugin_space, async_db_dump_hook, async_db_dump_desc);
	return 0;
}

int async_db_master_deinit() {
	aroop_assert(is_master());
	opp_hash_table_destroy(&global_db);
	pm_unplug_callback(0, async_db_CAS_hook);
	pm_unplug_callback(0, async_db_set_if_null_hook);
	pm_unplug_callback(0, async_db_unset_hook);
	pm_unplug_callback(0, async_db_get_hook);
	pm_unplug_callback(0, async_db_dump_hook);
	return 0;
}

C_CAPSULE_END

