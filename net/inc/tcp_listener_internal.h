#ifndef NGINZ_TCP_LISTENER_H
#define NGINZ_TCP_LISTENER_H

C_CAPSULE_START

int tcp_listener_init();
int tcp_listener_deinit();

int protostack_get_ports_internal(int*tcp_ports);

C_CAPSULE_END

#endif // NGINZ_TCP_LISTENER_H
