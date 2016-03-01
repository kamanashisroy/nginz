
#TARGET_BIN=nginz_main nginz_main_debug nginz_main_partitioned
TARGET_BIN=nginz_main_debug
TARGET_O=libnginz.o
TARGET_LAR=libnginz.la
TARGET_AR=libnginz.a libnginz_debug.a
bin_PROGRAMS=$(TARGET_BIN) $(TARGET_O)
lib_LTLIBRARIES=$(TARGET_LAR)
lib_LIBRARIES=$(TARGET_AR)
NGINZ_SOURCE_FILES = \
src/fiber.c     src/plugin.c	src/plugin_manager.c	src/scanner.c \
src/db.c \
src/binary_coder.c \
src/backtrace.c \
src/net/streamio.c \
src/shake.c	src/shake/help.c	src/shake/test.c	src/shake/quitall.c \
src/parallel/pipeline.c	src/parallel/fork.c	src/parallel/ping.c \
src/net/tcp_listener.c src/net/protostack.c \
src/net/chat.c	src/net/chat/chat_plugin_manager.c	src/net/chat/welcome.c	src/net/chat/room.c	src/net/chat/quit.c	src/net/chat/join.c	src/net/chat/broadcast.c	src/net/chat/leave.c	src/net/chat/user.c	src/net/chat/hiddenjoin.c	src/net/chat/uptime.c	src/net/chat/profiler.c	src/net/chat/chat_accept.c	src/net/chat/chat_factory.c	src/net/chat/chat_command.c	src/net/chat/version.c	src/net/chat/private_message.c \
src/net/http.c	src/net/http/http_accept.c	src/net/http/http_factory.c	src/net/http/http_parser.c	src/net/http/http_plugin_manager.c	src/net/http/http_tunnel.c \
src/apps/web_chat/web_chat.c	src/apps/web_chat/page_chat.c	src/apps/web_chat/web_session_factory.c \
src/parallel/async_db.c

EVENT_LOOP_POLL = src/event_loop_poll.c
EVENT_LOOP_PARTITIONED_OPENMP_POLL = src/event_loop_partitioned_openmp_poll.c
pkgincludedir=$(includedir)/nginz@PACKAGE_SUFFIX@
nobase_pkginclude_HEADERS= \
inc/event_loop.h \
inc/fiber.h       inc/plugin.h	inc/plugin_manager.h	inc/scanner.h \
inc/net/chat.h inc/net/chat/welcome.h

GIT_COMMIT_VERSION=`git describe --dirty --always --tags`

NGINZ_INCLUDES=-Iinc
NGINZ_CFLAGS=$(AROOP_CFLAGS) $(MEMCACHED_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\"

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
