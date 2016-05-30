
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
#include "log.h"
#include "event_loop.h"
#include "../src/event_loop_poll.c"



C_CAPSULE_START

START_TEST (test_event_loop)
{
	int fd = 5000;
	int ncount = _i;
	int prev = event_loop_fd_count();
	while(ncount--) {
		event_loop_register_fd(fd+ncount, NULL, NULL, POLLIN);
	}
	ncount = _i;
	while(ncount--) {
		event_loop_unregister_fd(fd+ncount);
	}
	event_loop_batch_unregister();
	
	ck_assert_int_eq(prev, event_loop_fd_count());
}
END_TEST

Suite * opp_factory_create_suite(void) {
	Suite *s;
	TCase *tc_core;
	s = suite_create("event_loop*.c");

	/* Core test case */
	tc_core = tcase_create("Core");

	nginz_core_init();
	tcase_add_loop_test(tc_core, test_event_loop, 1, 32);
	tcase_add_test(tc_core, test_event_loop);
	suite_add_tcase(s, tc_core);
	nginz_core_deinit();

	return s;
}

static int status = EXIT_FAILURE;
static int test_main() {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = opp_factory_create_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	status = ((number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, test_main);
	return status;
}

C_CAPSULE_END

