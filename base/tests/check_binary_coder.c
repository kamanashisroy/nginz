
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
#include "binary_coder.h"



C_CAPSULE_START

START_TEST (test_binary_coder)
{
	unsigned int expval = _i;
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset(&bin);
	binary_pack_int(&bin, expval);
	binary_pack_int(&bin, expval);
	aroop_txt_t str = {};
	aroop_txt_embeded_set_static_string(&str, "test"); 
	binary_pack_string(&bin, &str);
	//binary_coder_debug_dump(&bin);

	int intval = 0;
	int intval2 = 0;
	aroop_txt_t strval = {};
	binary_unpack_int(&bin, 0, &intval);
	ck_assert_int_eq(intval, expval);
	binary_unpack_int(&bin, 1, &intval2);
	ck_assert_int_eq(intval2, expval);
	binary_unpack_string(&bin, 2, &strval);
	
	aroop_txt_zero_terminate(&strval);
	aroop_txt_zero_terminate(&str);
	ck_assert(aroop_txt_equals(&strval, &str));
	//printf(" [%s!=%s] and [%d!=%d]\n", aroop_txt_to_string(&strval), aroop_txt_to_string(&str), intval, expval);
}
END_TEST

Suite * binary_coder_suite(void) {
	Suite *s;
	TCase *tc_core;
	s = suite_create("binary_coder.c");

	/* base test case */
	tc_core = tcase_create("base");

	nginz_core_init();
	tcase_add_loop_test(tc_core, test_binary_coder, 1, 100);
	suite_add_tcase(s, tc_core);
	nginz_core_deinit();

	return s;
}

static int status = EXIT_FAILURE;
static int test_main() {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = binary_coder_suite();
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

