
#include <sys/socket.h>
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "log.h"
#include "net/protostack.h"

C_CAPSULE_START

static int internal_port_for_stacks[NGINZ_MAX_PROTO] = {0};
struct protostack*internal_stacks[NGINZ_MAX_PROTO] = {NULL};
struct protostack*protostack_get(int port) {
	int i = 0;
	int count = sizeof(internal_stacks)/sizeof(*internal_stacks);
	for(i = 0; i < count; i++) {
		if(internal_port_for_stacks[i] != port)
			continue;
		return internal_stacks[i];
	}
	return NULL;
}

int protostack_set(int port, struct protostack*x) {
	int i = 0;
	int count = sizeof(internal_stacks)/sizeof(*internal_stacks);
	if(internal_stacks[count-1]) {
		syslog(LOG_ERR, "Protostack capacity is full\n");
		return -1;
	}
	for(i = 0; i < count; i++) {
		if(internal_stacks[i])
			continue;
		internal_port_for_stacks[i] = port;
		internal_stacks[i] = x;
	}
	return 0;
}

C_CAPSULE_END


