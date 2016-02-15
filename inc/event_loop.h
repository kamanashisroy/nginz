#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

C_CAPSULE_START

#include <poll.h>
/**
 * XXX this poll limits the callback for a certain fd to only one. 
 */
/**
 * @param requested_events the events to listen for
 */
int event_loop_register_fd(int fd, int (*on_event)(int fd, int returned_events, const void*event_data), const void*event_data, short requested_events);
int event_loop_unregister_fd(int fd); // NOTE the implementation is costly (may be it is better to do batch operation ..)

int event_loop_fd_count();

int event_loop_module_init();
int event_loop_module_deinit();

C_CAPSULE_END

#endif // EVENT_LOOP_H
