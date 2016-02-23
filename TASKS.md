
Tasks
========

- [x] Quit all child process.
- [x] Do soft-quit to logoff and disconnect all the users.
- [ ] Load test and report the result.
- [x] Write a version command.

### Aroop
- [x] Turn off the HAS_THREAD from aroop core. We do not need threading or locking.
- [ ] Use OPP_PFACTORY_CREATE_FULL to tune all the flags.

### net
- [x] Add HTTP support.
- [ ] add event_loop_epoll.c implementation based on epoll .
- [ ] add event_loop_poll_openmp.c implementation based on partitioned polling.

### parallel

### chat
- [x] Limit the username and chatroom length.
- [x] Limit user input length.
- [x] Disallow hidden command from user.
- [ ] Add uname command
- [x] Add uptime command
- [x] Show number of users served
- [x] Add profiler command to show memory profiler
- [x] Add HTTP support for chat
	- [x] Create webchat interface
- [ ] Write pidgin plugin

### shake
- [ ] Create memory profiler command to show memory usage.
- [ ] Make it non-blocking ..

### logging
- [x] Use syslog.

