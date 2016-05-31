
TARGET_BIN=nginz_net_main
TARGET_LAR=libnginz_net.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINZ_NET_SOURCE_FILES = \
src/net_subsystem.c \
src/streamio.c \
src/tcp_listener.c	src/protostack.c

NGINZ_BASE_LDFLAGS=$(top_builddir)/base/libnginz_base.la

NGINZ_NET_INCLUDES=-Iinc -I../base/inc
NGINZ_CFLAGS=$(AROOP_CFLAGS) $(MEMCACHED_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
libnginz_net_la_SOURCES = $(NGINZ_NET_SOURCE_FILES)
libnginz_net_la_CPPFLAGS = $(NGINZ_CFLAGS) $(NGINZ_NET_INCLUDES) 
libnginz_net_la_LDFLAGS = $(NGINZ_BASE_LDFLAGS)

nginz_net_main_SOURCES = $(NGINZ_NET_SOURCE_FILES)
nginz_net_main_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_NET_INCLUDES) 
nginz_net_main_LDFLAGS= -static @AROOP_DEBUG_LIBS@ $(MEMCACHED_LIBS) $(NGINZ_BASE_LDFLAGS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginz_net@PACKAGE_SUFFIX@.pc

libnginz_net@PACKAGE_SUFFIX@.pc: libnginz_net.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
