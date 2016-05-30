
TARGET_BIN=nginz_main_debug
TARGET_LAR=libnginz_chat.la
TARGET_AR=libnginz_chat.a libnginz_chat_debug.a
lib_LIBRARIES=$(TARGET_AR)
NGINZ_NET_SOURCE_FILES = \
src/net/chat.c	src/net/chat/chat_plugin_manager.c	src/net/chat/welcome.c	src/net/chat/room.c	src/net/chat/quit.c	src/net/chat/join.c	src/net/chat/broadcast.c	src/net/chat/leave.c	src/net/chat/user.c	src/net/chat/hiddenjoin.c	src/net/chat/uptime.c	src/net/chat/profiler.c	src/net/chat/chat_accept.c	src/net/chat/chat_factory.c	src/net/chat/chat_zombie.c	src/net/chat/chat_command.c	src/net/chat/version.c	src/net/chat/private_message.c	src/net/chat/room_master.c

NGINZ_NET_INCLUDES=-Iinc -I../core/inc
NGINZ_CFLAGS=$(AROOP_CFLAGS) $(MEMCACHED_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
libnginz_chat_debug_a_SOURCES = $(NGINZ_NET_SOURCE_FILES)
libnginz_chat_debug_a_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_NET_INCLUDES) 
libnginz_chat_a_SOURCES = $(NGINZ_NET_SOURCE_FILES)
libnginz_chat_a_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_NET_INCLUDES) 

# binary executables
nginz_core_main_SOURCES= $(NGINZ_SOURCE_FILES) src/main.c $(EVENT_LOOP_POLL)
nginz_core_main_CPPFLAGS= $(NGINZ_CFLAGS) -O2 $(NGINZ_INCLUDES)
nginz_core_main_LDFLAGS= $(AROOP_LIBS) $(MEMCACHED_LIBS)


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
pkgconfig_DATA = libnginz_chat@PACKAGE_SUFFIX@.pc

libnginz_chat@PACKAGE_SUFFIX@.pc: libnginz_chat.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
