
Tasks
========

- [x] Quit all child process.
- [x] Do soft-quit to logoff and disconnect all the users.
- [x] Write a version command.

### Aroop
- [x] Turn off the HAS_THREAD from aroop core. We do not need threading or locking.
- [x] Use OPP_PFACTORY_CREATE_FULL to tune all the flags.

### Packaging
- [ ] Use bii-code.
	- [ ] It needs cmake support.

### base
- [ ] Move memory profiler from chat module and let chat module use this profiler.
	- [ ] Write a document on memory profiler and its usage.

### parallel
- [x] Add support for data-type in binary coder.
- [x] Add support for mailbox.
- [x] Try to implement STAR topology instead of ring.(Hurry ! We reached 10K concurrency by doing this)
- [ ] Explain parallel architecture in state diagram.

### shake
- [ ] Create memory profiler command to show memory usage.
- [ ] Make it non-blocking ..

### logging
- [x] Use syslog.

### Testing
- [x] Load test and report the result.
- [x] Add unit test for base module.
- [ ] Add unit test for net.
- [ ] Add unit test for http.
- [ ] Add unit test for chat.
- [x] Add unit test for db.

### net
- [x] Add HTTP support.
- [ ] add event_loop_epoll.c implementation based on epoll .
- [ ] add event_loop_poll_openmp.c implementation based on partitioned polling.

#### Asynchronous API
- [ ] Add callback based line reader API. Use line reader in HTTP implementation.
- [ ] Make Send() network data asynchronous.
- [ ] Add services API
	- [ ] Add a REST-API

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
- [x] Add support for late destruction in zombie list.

#### chat rooms
- [x] Restrict users in chat room.

### DB
- [x] Write own db. And remove dependency from memcached.

### Scaling
- [ ] Add support for auto-scaling.

