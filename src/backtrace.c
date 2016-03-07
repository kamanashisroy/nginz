
#include <execinfo.h>
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_str2.h"
#include "nginz_config.h"
#include "backtrace.h"

C_CAPSULE_START

int print_stack_trace(aroop_txt_t*output) { // needs cleanup
	void *array[10];
	size_t size;
	char **strings;
	size_t i;
	aroop_assert(output != NULL);

	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);

	aroop_txt_embeded_buffer(output, 1024);
	aroop_txt_concat_string(output, "Stack trace:\n");
	//printf ("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++) {
		aroop_txt_concat_string (output, strings[i]);
		aroop_txt_concat_char (output, '\n');
	}

	free (strings);
	return 0;
}

C_CAPSULE_END

