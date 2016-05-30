
SUBDIRS= tests

TARGET_BIN=nginz_base_main
TARGET_LAR=libnginz_base.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINZ_SOURCE_FILES = \
src/fiber.c     src/plugin.c	src/plugin_manager.c	src/scanner.c \
src/binary_coder_extended.c \
src/backtrace.c \
src/shake.c	src/shake/help.c	src/shake/test.c	src/shake/quitall.c	src/shake/enumerate.c \
src/parallel/pipeline.c	src/parallel/fork.c	src/parallel/ping.c \
src/parallel/async_request.c \
src/base_subsystem.c

EVENT_LOOP_POLL = src/event_loop_poll.c
EVENT_LOOP_PARTITIONED_OPENMP_POLL = src/event_loop_partitioned_openmp_poll.c

pkgincludedir=$(includedir)/nginz_base@PACKAGE_SUFFIX@
nobase_pkginclude_HEADERS= \
inc/event_loop.h \
inc/fiber.h       inc/plugin.h	inc/plugin_manager.h	inc/scanner.h

GIT_COMMIT_VERSION=`git describe --dirty --always --tags`

NGINZ_INCLUDES=-Iinc
NGINZ_CFLAGS=$(AROOP_CFLAGS) $(MEMCACHED_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall

libnginz_base_la_SOURCES = $(NGINZ_SOURCE_FILES) $(EVENT_LOOP_POLL)
libnginz_base_la_CPPFLAGS = $(NGINZ_CFLAGS) $(NGINZ_INCLUDES)

# binary executables
nginz_base_main_SOURCES= $(NGINZ_SOURCE_FILES) src/main.c $(EVENT_LOOP_POLL)
nginz_base_main_CPPFLAGS= $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_INCLUDES)
nginz_base_main_LDFLAGS= -static @AROOP_DEBUG_LIBS@ $(MEMCACHED_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginz_base@PACKAGE_SUFFIX@.pc

libnginz_base@PACKAGE_SUFFIX@.pc: libnginz_base.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
