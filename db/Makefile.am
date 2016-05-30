
#TARGET_BIN=nginz_main nginz_main_debug nginz_main_partitioned
TARGET_BIN=nginz_main_debug
TARGET_O=libnginz.o
TARGET_LAR=libnginz.la
TARGET_AR=libnginz.a libnginz_debug.a
bin_PROGRAMS=$(TARGET_BIN) $(TARGET_O)
lib_LTLIBRARIES=$(TARGET_LAR)
lib_LIBRARIES=$(TARGET_AR)
NGINZ_SOURCE_FILES = \
src/parallel/async_db.c	src/parallel/async_db_master.c

EVENT_LOOP_POLL = src/event_loop_poll.c
EVENT_LOOP_PARTITIONED_OPENMP_POLL = src/event_loop_partitioned_openmp_poll.c
pkgincludedir=$(includedir)/nginz@PACKAGE_SUFFIX@
nobase_pkginclude_HEADERS= \
inc/event_loop.h \
inc/fiber.h       inc/plugin.h	inc/plugin_manager.h	inc/scanner.h

GIT_COMMIT_VERSION=`git describe --dirty --always --tags`

NGINZ_INCLUDES=-Iinc
NGINZ_CFLAGS=$(AROOP_CFLAGS) $(MEMCACHED_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall

libnginz_la_SOURCES = $(NGINZ_SOURCE_FILES) $(EVENT_LOOP_POLL)
libnginz_la_CPPFLAGS = $(NGINZ_CFLAGS) $(NGINZ_INCLUDES)
libnginz_debug_a_SOURCES = $(NGINZ_SOURCE_FILES) $(EVENT_LOOP_POLL)
libnginz_debug_a_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_INCLUDES) 
#libnginz_debug_a_CPPFLAGS = -ggdb3 -DAROOP_OPP_PROFILE -DAROOP_OPP_DEBUG -DMTRACE $(NGINZ_INCLUDES) 
libnginz_o_SOURCES = $(NGINZ_SOURCE_FILES) $(EVENT_LOOP_POLL)
libnginz_o_CPPFLAGS = $(NGINZ_CFLAGS) $(NGINZ_INCLUDES)
nginz_main_SOURCES= $(NGINZ_SOURCE_FILES) src/main.c $(EVENT_LOOP_POLL)
nginz_main_CPPFLAGS= $(NGINZ_CFLAGS) -O2 $(NGINZ_INCLUDES)
nginz_main_LDFLAGS= $(AROOP_LIBS) $(MEMCACHED_LIBS)
# NOTE for debugging enable the following
nginz_main_debug_SOURCES= $(NGINZ_SOURCE_FILES) src/main.c $(EVENT_LOOP_POLL)
#nginz_main_debug_CPPFLAGS= $(NGINZ_CFLAGS) --coverage -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_INCLUDES)
nginz_main_debug_CPPFLAGS= $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_INCLUDES)
#nginz_main_debug_LDFLAGS= $(AROOP_DEBUG_LIBS) --coverage $(MEMCACHED_LIBS)
nginz_main_debug_LDFLAGS= $(AROOP_DEBUG_LIBS) $(MEMCACHED_LIBS)
nginz_main_partitioned_SOURCES= $(NGINZ_SOURCE_FILES) src/main.c $(EVENT_LOOP_PARTITIONED_OPENMP_POLL)
nginz_main_partitioned_CPPFLAGS= $(NGINZ_CFLAGS) -fopenmp -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_INCLUDES)
nginz_main_partitioned_LDFLAGS= $(AROOP_DEBUG_LIBS) -lgomp $(MEMCACHED_LIBS)



pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginz@PACKAGE_SUFFIX@.pc

libnginz@PACKAGE_SUFFIX@.pc: libnginz.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
