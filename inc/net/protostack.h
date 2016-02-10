#ifndef NGINZ_PROTOSTACK_H
#define NGINZ_PROTOSTACK_H

C_CAPSULE_START

struct protostack {
	int (*on_connect)(int fd);
};
struct protostack*protostack_get(int port);
int protostack_set(int port, struct protostack*x);

C_CAPSULE_END

#endif // NGINZ_PROTOSTACK_H
