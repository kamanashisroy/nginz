
NginZ
==========

NginZ is a scalable application server. It is useful for instant messaging and audio/video communication.

Note
====

Plase update the port in inc/nginz\_config.h

Requirements
============

NginZ depends on the following packages,

- automake
- libtool
- pkg-config
- [Aroop Core](https://github.com/kamanashisroy/aroop_core)
- libmemcached

It it tested in Linux platform.

Building
========

The compilation command is. `./autogen.sh;make;make install;`

And the output binary is `nginz_main`, `nginz_debug_main`, `nginz_profiler_main`

Features
========

NginZ is equiped to serve as communication applications. It has,

- [Plugin](src/plugin.c) and dependency injection.
- Parallel processing support based on [token-ring and pipeline pattern](src/parallel/pipeline.c).
- It has scalability features. The requests are load-balanced in the worker processes.
- It has [memory profiler](src/net/chat/profiler.c).
- It has [event-loop](src/event_loop.c) module to handle user data in [fibers](src/fiber.c).
- It has [command shell](src/shake.c) to diagnose the server.
- Writing new feature for chat server needs very less code(see the following ..). 
- It has an HTTP interface(It is useful for benchmarking).

Dependency injection
====================

As a framework it supports dependency injection by plugin-spaces/extension-points. It makes less coupling and strong cohesion possible. And the result is less code and easily maintainable source for each features.

| Module/Source | Lines of code |
| --- | --- |
| ./src/net/chat/room.c | 182 |
| ./src/net/chat/quit.c | 43 |
| ./src/net/chat/leave.c | 34 |
| ./src/net/chat/broadcast.c | 182 |
| ./src/net/chat/join.c | 116 |
| ./src/net/chat/uptime.c | 48 |
| ./src/net/chat/profiler.c | 51 |
| ./src/net/chat/chat\_plugin\_manager.c | 118 |
| ./src/net/chat/welcome.c | 86 |
| ./src/net/chat/hiddenjoin.c | 75 |
| ./src/net/chat/user.c | 56 |

Module Description
===================

- [Chat Module](src/net/chat/README.md)
- [HTTP Module](src/net/http/README.md)

livedemo
========

```
telnet ec2-54-191-149-216.us-west-2.compute.amazonaws.com 9399
Trying 54.191.149.216...
Connected to ec2-54-191-149-216.us-west-2.compute.amazonaws.com.
Escape character is '^]'.
Welcome to NginZ chat server
Login name?
foo
Welcome foo!
/help
...
/rooms
Active rooms are:
	* ONE (1)
	* TWO
	* THREE
	* FOUR
	* FIVE
end of list.
/join ONE
Trying ...(14364)
Entering room:ONE
	* Shuva
	* foo(** this is you)
end of list
Shuva:hi foo
Shuva:I have to leave
Shuva:Have a nice day
	* user has left chat:Shuva
/uptime
...
/profiler
...
/quit
	* user has left chat:foo
BYE
Connection closed by foreign host.
```

Command shell
=============

```
socat - UNIX-CLIENT:/tmp/nginz.sock
help
plugin	shake	shake/plugin	src/plugin_manager.c
It dumps the avilable plugins
....
```

Benchmarking
============

I did benchmarking with [10K concurrency and 1 Million requests](BENCHMARKING.md). 

I did benchmarking in the localhost with 1K concurency.

```
ab -r -n 100000 -c 1000 http://localhost:80/
...
...
Concurrency Level:      1000
Time taken for tests:   5.975 seconds
Complete requests:      100000
Failed requests:        0
Write errors:           0
Total transferred:      4900000 bytes
HTML transferred:       1100000 bytes
Requests per second:    16736.06 [#/sec] (mean)
Time per request:       59.751 [ms] (mean)
Time per request:       0.060 [ms] (mean, across all concurrent requests)
Transfer rate:          800.85 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:       11   36 123.0     21    1031
Processing:     7   23   4.6     22      50
Waiting:        2   17   4.7     16      40
Total:         28   59 123.4     43    1061

Percentage of the requests served within a certain time (ms)
  50%     43
  66%     45
  75%     46
  80%     47
  90%     51
  95%     54
  98%     62
  99%   1040
 100%   1061 (longest request)
```
The result shows it can handle concurrent request. And the processor usage is uniform.

Tasks
======

Please refer to [tasks](TASKS.md) page.
