
NginZ
==========

NginZ is a scalable application server. It is useful for instant messaging and audio/video communication.

Note
====

Please update the port in `inc/nginz_config.h` to the appropriate value.

Requirements
============

NginZ depends on the following packages,

- automake
- libtool
- pkg-config
- [check](https://libcheck.github.io/check/)
- [Aroop Core](https://github.com/kamanashisroy/aroop_core)
- libmemcached(not needed anymore)

It it tested in Linux platform.

Building
========

The compilation command is. `./autogen.sh;make;make install;`

And the output binary is in apps directory.

```
find -iname "nginz_*_main"
apps/http/nginz_http_main
apps/chat/nginz_chat_main
```

The `nginz_http_main` listens to 80 port and typically a demo http server. The `nginz_chat_main` is a text messaging server(see below).

Features
========

NginZ is equiped to serve as communication applications. It has,

- [Plugin](base/plugin.md) and dependency injection.
- Parallel processing support based on [star-topology and pipeline pattern](base/src/parallel/pipeline.c). [This is elaborated in great details by the networking scenarios here.](base/parallel.md).
- It has scalability features. The requests are [load-balanced](apps/load_balancer) in the worker processes.
- It has [memory profiler](apps/chat/src/profiler.c).
- It has [event-loop](base/src/event_loop_poll.c) module to handle user data in [fibers](base/src/fiber.c).
- It has [command shell](base/src/shake.c) to diagnose the server.
- Writing new feature for chat server needs very less code(see the following ..). 
- It has an [HTTP interface](apps/http)(It is useful for benchmarking).
- It has [streamio](net/src/streamio.c) which supports io chaining. It is useful to implement proxy-pattern and chain-of-responsiblity pattern. The instant-messaging(chat) server is tunneled through http protocol using this feature.

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

- [Chat Module](apps/chat/README.md)
- [HTTP Module](apps/http/README.md)
- [Parallel Processing Module](base/parallel.md)

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

Webchat
========
The nginz http uses session to tunnel state-ful chat server.

The [web-chat live demo prototype is available here](http://ec2-54-191-149-216.us-west-2.compute.amazonaws.com/pagechat).

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

I did benchmarking in the localhost with 10K concurency.

```
ab -r -n 100000 -c 10000 http://localhost:80/
This is ApacheBench, Version 2.3 <$Revision: 655654 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        
Server Hostname:        localhost
Server Port:            80

Document Path:          /
Document Length:        11 bytes

Concurrency Level:      10000
Time taken for tests:   10.617 seconds
Complete requests:      100000
Failed requests:        0
Write errors:           0
Total transferred:      4900000 bytes
HTML transferred:       1100000 bytes
Requests per second:    9418.73 [#/sec] (mean)
Time per request:       1061.714 [ms] (mean)
Time per request:       0.106 [ms] (mean, across all concurrent requests)
Transfer rate:          450.70 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:      117  579 683.4    416    7497
Processing:   118  392 137.4    399    7036
Waiting:       16  236 156.3    214    7003
Total:        345  971 715.9    813   10371

Percentage of the requests served within a certain time (ms)
  50%    813
  66%    907
  75%    969
  80%    987
  90%   1649
  95%   1723
  98%   2136
  99%   3797
 100%  10371 (longest request)
```
The result shows it can handle concurrent request. And the processor usage is uniform.

Tasks
======

Please refer to [tasks](TASKS.md) page.

Debug notes
===========

For server crash, use

```
addr2line -e ./nginz_main_debug ip
```
