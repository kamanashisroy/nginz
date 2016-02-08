
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

int binary_coder_reset(aroop_txt_t*buffer) {
	aroop_txt_set_len(buffer, 0);
	aroop_txt_concat_char(1);
	return 0;
}

int binary_concat_string(aroop_txt_t*buffer, aroop_txt_t*x) {
	int blen = aroop_txt_length(buffer);
	if((aroop_txt_length(x) + blen + 1) > 255)
		return -1;
	aroop_txt_concat_char(buffer, (unsigned char)aroop_txt_length(x));
	aroop_txt_concat(buffer, x);
	blen = aroop_txt_length(buffer);
	aroop_txt_set_char_at(buffer, 0, blen);
	return 0;
}

C_CAPSULE_END


