
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



C_CAPSULE_START

static int remaining = 101;
static int step_100_before_quit(int status) {
	remaining--;
	if(!remaining)
		fiber_quit();
	return 0;
}

START_TEST (test_pipeline)
{
	ck_assert_int_eq(1, 1);
}
END_TEST

Suite * pipeline_suite(void) {
	TCase *tc_core;
	Suite *s = suite_create("parallel/pipeline.c");
	/* base test case */
	tc_core = tcase_create("base");
	tcase_add_test(tc_core, test_pipeline);
	suite_add_tcase(s, tc_core);
	return s;
}

static int status = EXIT_FAILURE;
static int test_main() {
	int number_failed;
	/* initiate nginz */
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz_parallel_check", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	nginz_core_init();
	/* initiate fibers */
	register_fiber(step_100_before_quit);
	nginz_parallel_init();

	/* cleanup nginz */
	fiber_module_run();
	nginz_core_deinit();
	closelog();

	SRunner *sr = srunner_create(pipeline_suite());
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

