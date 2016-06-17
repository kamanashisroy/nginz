
/*
 * This file part of aroop.
 *
 * Copyright (C) 2012  Kamanashis Roy
 *
 * Aroop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MiniIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Aroop.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: Feb 9, 2011
 *  Author: Kamanashis Roy (kamanashisroy@gmail.com)
 */

#include <check.h>
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "nginz_config.h"
#include "log.h"
#include "fiber.h"
#include "fiber_internal.h"
#include "parallel/pipeline.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "async_db.h"



C_CAPSULE_START

#define MAGIC_NUMBER 121
static int cb_test_set_value = 0;
static int cb_test_token_passed = 0;
//#include "../src/parallel/pipeline.c"
static int set_token(const int token) {
	aroop_txt_t key = {};
	aroop_txt_embeded_set_static_string(&key, "db/test/key");
	cb_test_set_value = MAGIC_NUMBER;
	cb_test_token_passed = token;
	async_db_set_int(token, NULL, &key, cb_test_set_value);
	syslog(LOG_NOTICE, "[%d]set the key\n", token);
	return 0;
}

static int cb_test_token = 0;
static int cb_test_get_value = 0;
static int db_test_cb(aroop_txt_t*bin, aroop_txt_t*output) {
	int srcpid = 0;
	aroop_txt_t gval = {};
	syslog(LOG_NOTICE, "[%d]test callback is invoked\n", getpid());
	binary_unpack_int(bin, 0, &srcpid);
	syslog(LOG_NOTICE, "[%d]src:%d\n", getpid(), srcpid);
	binary_unpack_int(bin, 2, &cb_test_token); // this should be same as our pid
	syslog(LOG_NOTICE, "[%d]callback token:%d\n", getpid(), cb_test_token);
	binary_unpack_string(bin, 5, &gval); // needs cleanup
	cb_test_get_value = atoi(aroop_txt_to_string(&gval));
	syslog(LOG_NOTICE, "[%d]invoked src:%d, token:%d, value:%d\n", getpid(), srcpid, cb_test_token, cb_test_get_value);
	aroop_txt_destroy(&gval);
	return 0;
}

static int db_test_cb_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "dbtest", "db", plugin_space, __FILE__, "It is the callback invoked when there is reply.\n");
}

static int get_token(const int token) {
	aroop_txt_t key = {};
	aroop_txt_embeded_set_static_string(&key, "db/test/key");
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "db/test/cb");
	async_db_get(token, &plugin_space, &key);
	syslog(LOG_NOTICE, "[%d]sent get key\n", token);
	aroop_txt_embeded_set_static_string(&plugin_space, "db/test/cb");
	pm_plug_callback(&plugin_space, db_test_cb, db_test_cb_desc);
	return 0;
}


static int step_before_quit(int status) {
	static int remaining = 20;
	remaining--;
	if(remaining == 15 && !is_master()) {
		set_token(getpid());
	}
	if(remaining == 10 && !is_master()) {
		get_token(getpid());
	}
	if(!remaining)
		fiber_quit();
	return 0;
}

START_TEST (test_db_child)
{
	ck_assert(!is_master());
	ck_assert_int_eq(cb_test_token, cb_test_token_passed);
	ck_assert_int_eq(cb_test_get_value, MAGIC_NUMBER);
	ck_assert_int_eq(cb_test_set_value, MAGIC_NUMBER);
}
END_TEST

Suite * db_child_suite(void) {
	TCase *tc_core;
	Suite *s = suite_create("async_db.c");
	/* base test case */
	tc_core = tcase_create("db");
	tcase_add_test(tc_core, test_db_child);
	suite_add_tcase(s, tc_core);
	return s;
}

START_TEST (test_db_master)
{
	ck_assert(is_master());
	/* get the child test status */
	int child_status = 0;
	while(wait(&child_status) > 0) {
		ck_assert_int_eq(child_status, EXIT_SUCCESS);
	}
}
END_TEST



Suite * db_master_suite(void) {
	TCase *tc_core;
	Suite *s = suite_create("async_db.c");
	/* base test case */
	tc_core = tcase_create("db");
	tcase_add_test(tc_core, test_db_master);
	suite_add_tcase(s, tc_core);
	return s;
}

static int status = EXIT_SUCCESS;
static int test_main() {
	int number_failed;
	/* initiate nginz */
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz_db_check", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	nginz_core_init();
	/* initiate fibers */
	register_fiber(step_before_quit);
	nginz_db_module_init_before_parallel_init();
	nginz_parallel_init();
	nginz_db_module_init_after_parallel_init();

	/* cleanup nginz */
	fiber_module_run();
	nginz_db_module_deinit();
	nginz_core_deinit();
	closelog();

	if(is_master()) {
		SRunner *sr = srunner_create(db_master_suite());
		srunner_set_fork_status(sr, CK_NOFORK);
		srunner_run_all(sr, CK_NORMAL);
		number_failed = srunner_ntests_failed(sr);
		srunner_free(sr);
		status = ((number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
	} else {
		//syslog(LOG_NOTICE, "[%d]is master\n", getpid());
		SRunner *sr = srunner_create(db_child_suite());
		srunner_set_fork_status(sr, CK_NOFORK);
		srunner_run_all(sr, CK_NORMAL);
		number_failed = srunner_ntests_failed(sr);
		srunner_free(sr);
		status = ((number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
	}
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, test_main);
	return status;
}

C_CAPSULE_END

