
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "plugin.h"
#include "plugin_manager.h"
#include "binary_coder.h"

C_CAPSULE_START

typedef char mychar_t;
int binary_coder_reset(aroop_txt_t*buffer) {
	aroop_txt_set_length(buffer, 0);
	// base header at 0
	aroop_txt_concat_char(buffer, (mychar_t)1);
	aroop_assert(aroop_txt_capacity(buffer) >= 256);
	return 0;
}

int binary_coder_reset_for_pid(aroop_txt_t*buffer, int destpid) {
	aroop_assert(destpid > -1);
	binary_coder_reset(buffer);
	aroop_txt_t deststr = {};
	aroop_txt_embeded_stackbuffer(&deststr, 32);
	aroop_txt_printf(&deststr, "%d", destpid);
	binary_pack_string(buffer, &deststr);
	return 0;
}

int binary_pack_string(aroop_txt_t*buffer, aroop_txt_t*x) {
	mychar_t blen = (mychar_t)aroop_txt_length(buffer);
	if((aroop_txt_length(x) + blen + 1) > 255)
		return -1;
	// add header
	aroop_txt_concat_char(buffer, (mychar_t)aroop_txt_length(x));
	// add body
	aroop_txt_concat(buffer, x);
	blen = (mychar_t)aroop_txt_length(buffer);
	// update base header
	aroop_txt_set_char_at(buffer, 0, blen);
	return 0;
}

int binary_unpack_string(aroop_txt_t*buffer, int skip, aroop_txt_t*x) {
	mychar_t pos = 1;
	mychar_t blen = (mychar_t)aroop_txt_length(buffer);
	if(blen == 1)
		return 0;
	// do not trust the blen use my blen
	blen = (mychar_t)aroop_txt_char_at(buffer, 0);
	printf("given blen:%d, data available:%d\n", blen, aroop_txt_length(buffer));
	if(blen > (aroop_txt_length(buffer))) {
		printf("Error in buffer data, may be the socket is corrupted 1\n");
		return -1;
	}
	do {
		mychar_t nlen = (mychar_t)aroop_txt_char_at(buffer, pos);
		printf("pos %d, nlen %d, blen %d\n", pos, nlen, blen);
		if(pos + nlen > blen) {
			// error
			printf("Error in buffer data, may be the socket is corrupted 2\n");
			return -1;
		}
		aroop_txt_embeded_txt_copy_shallow(x,buffer);
		pos++;
		nlen;
		aroop_txt_shift(x, pos);
		aroop_txt_set_length(x, nlen);
		pos+= nlen;
		if(!skip)
			return 0;
		skip--;
	} while(pos < blen);
	return 0;
}


int binary_unpack_int(aroop_txt_t*buffer, int skip, int*intval) {
	aroop_txt_t x = {};
	binary_unpack_string(buffer, skip, &x);
	aroop_txt_t sandbox = {};
	aroop_txt_embeded_stackbuffer(&sandbox, 32);
	aroop_txt_concat(&sandbox, &x);
	aroop_txt_zero_terminate(&sandbox);
	printf(" pid = %s", aroop_txt_to_string(&sandbox));
	*intval = aroop_txt_to_int(&sandbox);
	aroop_txt_destroy(&x);
	return 0;
}


static int binary_coder_test(aroop_txt_t*input, aroop_txt_t*output) {
	int expval = 32;
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, expval);
	aroop_txt_t str = {};
	aroop_txt_embeded_set_static_string(&str, "test"); 
	binary_pack_string(&bin, &str);

	int intval = 0;
	aroop_txt_t strval = {};
	binary_unpack_int(&bin, 0, &intval);
	binary_unpack_string(&bin, 1, &strval);
	
	int success = (intval == expval && aroop_txt_equals(&strval, &str));

	aroop_txt_embeded_buffer(output, 512);
	if(success) {
		aroop_txt_concat_string(output, "successful\n");
	} else {
		aroop_txt_zero_terminate(&strval);
		aroop_txt_zero_terminate(&str);
		aroop_txt_printf(output, "FAILED [%s!=%s] and [%d!=%d]\n", aroop_txt_to_string(&strval), aroop_txt_to_string(&str), intval, expval);
	}
	return 0;
}

static int binary_coder_test_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "binary_coder_test", "test", plugin_space, __FILE__, "It is test code for binary coder.\n");
}


int binary_coder_module_init() {
	aroop_txt_t binary_coder_plug;
	aroop_txt_embeded_set_static_string(&binary_coder_plug, "test/binary_coder_test");
	pm_plug_callback(&binary_coder_plug, binary_coder_test, binary_coder_test_desc);
}

int binary_coder_module_deinit() {
	pm_unplug_callback(0, binary_coder_test);
}

C_CAPSULE_END


