
#include <sys/socket.h>
#include <aroop/aroop_core.h>
#include "nginz_config.h"
#include "net/protostack.h"

C_CAPSULE_START

struct protostack*defaultstack = NULL;
struct protostack*protostack_get(int port) {
	return defaultstack;
}

int protostack_set(int port, struct protostack*x) {
	defaultstack = x;
	return 0;
}

C_CAPSULE_END


