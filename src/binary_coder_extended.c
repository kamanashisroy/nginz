
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "binary_coder.h"

C_CAPSULE_START

enum {
	NGINZ_BINARY_CONTENT_NULL = 0,
	NGINZ_BINARY_CONTENT_STRING,
	NGINZ_BINARY_CONTENT_INTEGER,
};

enum {
	BINARY_CODER_HEADER_LEN = 3,
};

typedef char mychar_t;
int binary_coder_reset(aroop_txt_t*buffer) {
	aroop_txt_set_length(buffer, 0);
	// add base header [0,2]
	// NOTE unlike content header which is BINARY_CODER_HEADER_LEN, base header is (BINARY_CODER_HEADER_LEN-1) bytes.
	aroop_txt_concat_char(buffer, (mychar_t)0);
	aroop_txt_concat_char(buffer, (mychar_t)2);
	aroop_assert(aroop_txt_capacity(buffer) >= 256);
	return 0;
}

int binary_coder_reset_for_pid(aroop_txt_t*buffer, int destpid) {
	aroop_assert(destpid > -1);
	binary_coder_reset(buffer);
	binary_pack_int(buffer, destpid);
	return 0;
}

static int binary_set_header(char*header, mychar_t content_type, uint16_t len) {
	header[0] = content_type;
	header[1] = (mychar_t)((len & 0xFF00)>>8);
	header[2] = (mychar_t)(len & 0x00FF);
	return 0;
}

static int binary_pack_string_helper(aroop_txt_t*buffer, aroop_txt_t*x, mychar_t content_type) {
	mychar_t blen = (mychar_t)aroop_txt_length(buffer);
	if((aroop_txt_length(x) + blen + BINARY_CODER_HEADER_LEN) > aroop_txt_capacity(buffer)) {
		syslog(LOG_ERR, "Binary buffer is full");
		return -1;
	}
	// add header
	uint16_t len = aroop_txt_length(x);
	mychar_t str[3];
	binary_set_header(str, content_type, len);
	aroop_txt_concat_char(buffer, str[0]);
	aroop_txt_concat_char(buffer, str[1]);
	aroop_txt_concat_char(buffer, str[2]);
	// add body
	aroop_txt_concat(buffer, x);
	blen = (mychar_t)aroop_txt_length(buffer);
	// update base header
	str[0] = (mychar_t)((blen & 0xFF00)>>8);
	str[1] = (mychar_t)(blen & 0x00FF);
	aroop_txt_set_char_at(buffer, 0, str[0]);
	aroop_txt_set_char_at(buffer, 1, str[1]);
	return 0;
}

static aroop_txt_t intbuf = {};
int binary_pack_int(aroop_txt_t*buffer, int intval) {
	aroop_txt_set_length(&intbuf, 1);
	char*str = aroop_txt_to_string(&intbuf);
	aroop_txt_set_length(&intbuf, 0);
	if(intval >= 0xFFFF) {
		//str[intbuf.len++] = /*(0<<6) |*/ 4; // 0 means numeral , 4 is the numeral size
		str[intbuf.len++] = (unsigned char)((intval & 0xFF000000)>>24);
		str[intbuf.len++] = (unsigned char)((intval & 0x00FF0000)>>16);
	//} else {
		//str[intbuf.len++] = /*(0<<6) |*/ 2; // 0 means numeral , 2 is the numeral size
	}
	str[intbuf.len++] = (unsigned char)((intval & 0xFF00)>>8);
	str[intbuf.len++] = (unsigned char)(intval & 0x00FF);

	//aroop_txt_embeded_stackbuffer(&str, 32);
	//aroop_txt_printf(&str, "%d", intval);
	return binary_pack_string_helper(buffer, &intbuf, NGINZ_BINARY_CONTENT_INTEGER);
}

int binary_pack_string(aroop_txt_t*buffer, aroop_txt_t*x) {
	return binary_pack_string_helper(buffer, x, NGINZ_BINARY_CONTENT_STRING);
}

static int binary_get_header(char*header, int offset, mychar_t*content_type, uint16_t*len) {
	*content_type = header[offset++];
	*len = (uint8_t)header[offset++];
	*len = (*len) << 8;
	*len |= ((uint8_t)header[offset]) & 0xFF;
	return 0;
}

static int binary_unpack_string_helper(aroop_txt_t*buffer, int skip, aroop_txt_t*x, mychar_t expected_content_type) {
	mychar_t pos = 2;
	mychar_t content_type = 0;
	uint16_t blen = aroop_txt_length(buffer);
	assert(blen >= (BINARY_CODER_HEADER_LEN-1));
	aroop_txt_destroy(x); // remove the old value
	if(blen == (BINARY_CODER_HEADER_LEN-1))
		return 0;
	// do not trust the blen use my blen
	blen = (mychar_t)aroop_txt_char_at(buffer, 0);
	blen = (blen) << 8;
	blen |= (mychar_t)aroop_txt_char_at(buffer, 1);
	if(blen > (aroop_txt_length(buffer))) {
		syslog(LOG_ERR, "Error in buffer data, may be the socket is corrupted 1\n");
		return -1;
	}
	do {
		uint16_t nlen = 0;
		binary_get_header(aroop_txt_to_string(buffer), pos, &content_type, &nlen);
		//syslog(LOG_NOTICE, "pos %d, nlen %d, blen %d\n", pos, nlen, blen);
		if((pos + nlen) > blen) {
			// error
			syslog(LOG_ERR, "Error in buffer data, may be the socket is corrupted (%d+%d)>%d\n", pos, nlen, blen);
			return -1;
		}
		if(skip) {
			pos += nlen+BINARY_CODER_HEADER_LEN;
			skip--;
			continue;
		}
		if(content_type != expected_content_type) {
			syslog(LOG_ERR, "Error mismatch in content type\n");
			aroop_assert(!"Error mismatch in content type\n");
			return -1;
		}
		aroop_txt_embeded_txt_copy_shallow(x,buffer); // needs cleanup
		pos+= BINARY_CODER_HEADER_LEN;
		aroop_txt_shift(x, pos);
		aroop_txt_set_length(x, nlen);
		break;
	} while(pos < blen);
	return 0;
}

int binary_unpack_string(aroop_txt_t*buffer, int skip, aroop_txt_t*x) {
	return binary_unpack_string_helper(buffer, skip, x, NGINZ_BINARY_CONTENT_STRING);
}

int binary_unpack_int(aroop_txt_t*buffer, int skip, int*intval) {
	aroop_txt_t x = {};
	if(binary_unpack_string_helper(buffer, skip, &x, NGINZ_BINARY_CONTENT_INTEGER)) {
		return -1;
	}
	//*intval = aroop_txt_to_int(&sandbox);
	int output = 0;
	int offset = 0;
	char*str = aroop_txt_to_string(&x);
	if(x.len >= 1) {
		output = (unsigned char)str[offset];
	}
	if(x.len >= 2) {
		output = output << 8;
		output |= ((unsigned char)str[offset+1]) & 0xFF;
	}
	if(x.len >= 3) {
		output = output << 8;
		output |= ((unsigned char)str[offset+2]) & 0xFF;
	}
	if(x.len == 4) {
		output = output << 8;
		output |= ((unsigned char)str[offset+3]) & 0xFF;
	}
	*intval = output;
	aroop_txt_destroy(&x);
	return 0;
}

int binary_coder_fixup(aroop_txt_t*buffer) {
	uint16_t blen = (mychar_t)aroop_txt_length(buffer);
	assert(blen >= (BINARY_CODER_HEADER_LEN-1));
	blen = (mychar_t)aroop_txt_char_at(buffer, 0);
	blen = (blen) << 8;
	blen |= (mychar_t)aroop_txt_char_at(buffer, 1);
	// do not trust the blen use my blen
	if(blen != aroop_txt_length(buffer)) {
		//printf("Fixing .. given blen:%d, data available:%d\n", blen, aroop_txt_length(buffer));
		aroop_txt_set_length(buffer, blen);
	}
	return 0;
}

int binary_coder_debug_dump(aroop_txt_t*buffer) {
	int skip = 0;
	do {
		aroop_txt_t x = {};
		if(binary_unpack_string(buffer, skip, &x)) {
			return -1;
		}
		skip++;
		if(aroop_txt_is_empty(&x))
			break;
		aroop_txt_t sandbox = {};
		aroop_txt_embeded_stackbuffer(&sandbox, 32);
		aroop_txt_concat(&sandbox, &x);
		aroop_txt_zero_terminate(&sandbox);
		printf("%s\n", aroop_txt_to_string(&sandbox));
		aroop_txt_destroy(&x);
	}while(1);
	return 0;
}

static int binary_coder_test_helper(int expval) {
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, expval);
	binary_pack_int(&bin, expval);
	aroop_txt_t str = {};
	aroop_txt_embeded_set_static_string(&str, "test"); 
	binary_pack_string(&bin, &str);
	binary_coder_debug_dump(&bin);

	int intval = 0;
	int intval2 = 0;
	aroop_txt_t strval = {};
	binary_unpack_int(&bin, 0, &intval);
	binary_unpack_int(&bin, 1, &intval2);
	binary_unpack_string(&bin, 2, &strval);
	
	aroop_txt_zero_terminate(&strval);
	aroop_txt_zero_terminate(&str);
	//printf(" [%s!=%s] and [%d!=%d]\n", aroop_txt_to_string(&strval), aroop_txt_to_string(&str), intval, expval);
	return !(intval == expval && intval2 == expval && aroop_txt_equals(&strval, &str));
}

static int binary_coder_test(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 512);
	if(binary_coder_test_helper(32) || binary_coder_test_helper(0)) {
		//aroop_txt_printf(output, "FAILED [%s!=%s] and [%d!=%d]\n", aroop_txt_to_string(&strval), aroop_txt_to_string(&str), intval, expval);
		aroop_txt_printf(output, "Binary coder: FAILED\n");
	} else {
		aroop_txt_concat_string(output, "Binary coder: successful\n");
	}
	return 0;
}

static int binary_coder_test_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "binary_coder_test", "test", plugin_space, __FILE__, "It is test code for binary coder.\n");
}

int binary_coder_module_init() {
	aroop_txt_embeded_buffer(&intbuf, 32);
	aroop_txt_t binary_coder_plug;
	aroop_txt_embeded_set_static_string(&binary_coder_plug, "test/binary_coder_test");
	pm_plug_callback(&binary_coder_plug, binary_coder_test, binary_coder_test_desc);
}

int binary_coder_module_deinit() {
	pm_unplug_callback(0, binary_coder_test);
}

C_CAPSULE_END


