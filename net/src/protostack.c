
#include <string.h>
#include <sys/socket.h>
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "log.h"
#include "protostack.h"

C_CAPSULE_START

static int internal_port_for_stacks[NGINZ_MAX_PROTO] = {0};
static struct protostack*internal_stacks[NGINZ_MAX_PROTO] = {NULL};
struct protostack*protostack_get(int port) {
	int i = 0;
	for(i = 0; i < NGINZ_MAX_PROTO; i++) {
		if(internal_port_for_stacks[i] != port)
			continue;
		return internal_stacks[i];
	}
	return NULL;
}

int protostack_set(int port, struct protostack*x) {
	int i = 0;
	if(internal_stacks[NGINZ_MAX_PROTO-1]) {
		syslog(LOG_ERR, "Protostack capacity is full\n");
		return -1;
	}
	for(i = 0; i < NGINZ_MAX_PROTO; i++) {
		if(internal_stacks[i])
			continue;
		syslog(LOG_INFO, "Registering protocol for port:%d\n", port);
		internal_port_for_stacks[i] = port;
		internal_stacks[i] = x;
		break;
	}
	return 0;
}

int protostack_get_ports_internal(int*tcp_ports) {
	int i = 0;
	//memset(tcp_ports, 0, sizeof(int)*NGINZ_MAX_PROTO);
	for(i = 0; i < NGINZ_MAX_PROTO; i++) {
		if(internal_stacks[i])
			tcp_ports[i] = internal_port_for_stacks[i];
		else
			break;
	}
	return 0;
}

int protostack_init() {
	memset(internal_port_for_stacks, 0, sizeof(internal_port_for_stacks));
	memset(internal_stacks, 0, sizeof(internal_stacks));
	return 0;
}

int protostack_deinit() {
	// nothing to do
	return 0;
}

C_CAPSULE_END


