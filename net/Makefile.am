
TARGET_LAR=libnginz_net.la
TARGET_AR=libnginz_net.a libnginz_net_debug.a
lib_LIBRARIES=$(TARGET_AR)
NGINZ_NET_SOURCE_FILES = \
src/net/streamio.c \
src/net/tcp_listener.c src/net/protostack.c

NGINZ_NET_INCLUDES=-Iinc -I../core/inc
NGINZ_CFLAGS=$(AROOP_CFLAGS) $(MEMCACHED_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
libnginz_net_debug_a_SOURCES = $(NGINZ_NET_SOURCE_FILES)
libnginz_net_debug_a_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_NET_INCLUDES) 
libnginz_net_a_SOURCES = $(NGINZ_NET_SOURCE_FILES)
libnginz_net_a_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_NET_INCLUDES) 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginz_net@PACKAGE_SUFFIX@.pc

libnginz_net@PACKAGE_SUFFIX@.pc: libnginz_net.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
