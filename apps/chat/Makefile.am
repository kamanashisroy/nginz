
TARGET_BIN=nginz_chat_main
TARGET_LAR=libnginz_chat.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINZ_CHAT_SOURCE_FILES = \
src/chat.c	src/chat_plugin_manager.c	src/welcome.c	src/room.c	src/quit.c	src/join.c	src/broadcast.c	src/leave.c	src/user.c	src/hiddenjoin.c	src/uptime.c	src/profiler.c	src/chat_accept.c	src/chat_factory.c	src/chat_zombie.c	src/chat_command.c	src/version.c	src/private_message.c	src/room_master.c

NGINZ_CHAT_LDFLAGS=$(top_builddir)/base/libnginz_base.la $(top_builddir)/net/libnginz_net.la $(top_builddir)/apps/load_balancer/libnginz_lb.la $(top_builddir)/db/libnginz_db.la
NGINZ_CHAT_INCLUDES=-Iinc -I$(top_builddir)/base/inc -I$(top_builddir)/net/inc -I$(top_builddir)/apps/load_balancer/inc  -I$(top_builddir)/db/inc 


NGINZ_CFLAGS=$(AROOP_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
libnginz_chat_la_SOURCES = $(NGINZ_CHAT_SOURCE_FILES)
libnginz_chat_la_CPPFLAGS = $(NGINZ_CFLAGS) $(NGINZ_CHAT_INCLUDES)
libnginz_chat_la_LDFLAGS = $(NGINZ_CHAT_LDFLAGS)

nginz_chat_main_SOURCES = $(NGINZ_CHAT_SOURCE_FILES) src/main.c
nginz_chat_main_CPPFLAGS = $(NGINZ_CFLAGS) -O2 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_CHAT_INCLUDES) 
nginz_chat_main_LDFLAGS= -static $(NGINZ_CHAT_LDFLAGS) @AROOP_DEBUG_LIBS@ 

#MAINTAINERCLEANFILES = nginz.pc
