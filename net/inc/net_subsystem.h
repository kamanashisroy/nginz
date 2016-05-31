#ifndef NGINZ_NET_SUBSYSTEM_H
#define NGINZ_NET_SUBSYSTEM_H

C_CAPSULE_START

int nginz_net_init();
int nginz_net_init_after_parallel_init();
int nginz_net_deinit();

C_CAPSULE_END

#endif // NGINZ_NET_SUBSYSTEM_H
