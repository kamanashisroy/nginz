
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

It it tested in Linux and Mac/OSx platform.

Building
========

The compilation command is. `./autogen.sh;make;make install;`

And the output binary is `nginz_main`, `nginz_debug_main`, `nginz_profiler_main`

Features
========

NginZ is equiped to serve as production applications. It has,

- Plugin and dependency injection.
- Parallel processing support based on token-ring and pipeline pattern.
- It has scalability features. The requests are load-balanced in the worker processes.
- It has memory profiler.

