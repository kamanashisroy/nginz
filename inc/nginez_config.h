#ifndef NGINEZ_CONFIG_H
#define NGINEZ_CONFIG_H

C_CAPSULE_START

#define MAX_POLL_FD 10000
#define NGINEZ_INLINE inline
#define NUMBER_OF_PROCESSORS 3
//#define NGINEZ_POLL_ALL_FLAGS POLLIN | POLLPRI | POLLHUP
#define NGINEZ_POLL_ALL_FLAGS POLLIN
#define NGINEZ_CHAT_PORT 9599

C_CAPSULE_END

#endif // NGINEZ_CONFIG_H
