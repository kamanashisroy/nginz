
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
#include "fiber.h"
#include "fiber_internal.h"



C_CAPSULE_START

static int test_single_fiber_is_run = 0;
static int suite_fiber(int status) {
	test_single_fiber_is_run = 1;
	fiber_quit();
	return 0;
}

START_TEST (test_single_fiber)
{
	ck_assert_int_eq(test_single_fiber_is_run, 1);
}
END_TEST

Suite * single_fiber_suite(void) {
	TCase *tc_core;
	Suite *s = suite_create("parallel/single_fiber.c");
	/* base test case */
	tc_core = tcase_create("base");
	tcase_add_test(tc_core, test_single_fiber);
	suite_add_tcase(s, tc_core);
	return s;
}

static int status = EXIT_FAILURE;
static int test_main() {
	int number_failed;
	/* initiate nginz */
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz_fiber_check", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	nginz_core_init();
	//nginz_parallel_init();

	/* initiate fibers */
	register_fiber(suite_fiber);

	/* cleanup nginz */
	fiber_module_run();
	nginz_core_deinit();
	closelog();

	SRunner *sr = srunner_create(single_fiber_suite());
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	if(test_single_fiber_is_run == 0) {
		number_failed++;
	}
	srunner_free(sr);
	status = ((number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, test_main);
	return status;
}

C_CAPSULE_END

