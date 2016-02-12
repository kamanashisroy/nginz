#ifndef NGINZ_PROTOSTACK_H
#define NGINZ_PROTOSTACK_H

C_CAPSULE_START

/**
 * This is State/Strategy pattern for protocol stack. It allows multiple stack to operate on one server.
 */

struct protostack {
	int (*on_guest_hookup)(int fd, aroop_txt_t*cmd);
};
struct protostack*protostack_get(int port);
int protostack_set(int port, struct protostack*x);

C_CAPSULE_END

#endif // NGINZ_PROTOSTACK_H
