
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
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
	do {
		mychar_t nlen = (mychar_t)aroop_txt_char_at(buffer, pos);
		if(pos + nlen > blen) {
			// error
			printf("Error in buffer data, may be the socket is corrupted\n");
			return -1;
		}
		aroop_txt_embeded_txt_copy_shallow(x,buffer);
		pos++;
		nlen--;
		aroop_txt_shift(x, pos);
		aroop_txt_set_length(x, nlen);
		pos+= nlen;
		if(!skip)
			return 0;
		skip--;
	} while(pos < blen);
	return 0;
}
C_CAPSULE_END


