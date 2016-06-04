
TARGET_BIN=nginz_http_main
TARGET_LAR=libnginz_http.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINZ_HTTP_SOURCE_FILES = \
src/http_subsystem.c	src/http_accept.c	src/http_factory.c	src/http_parser.c	src/http_plugin_manager.c	src/http_tunnel.c

NGINZ_HTTP_LDFLAGS=$(top_builddir)/base/libnginz_base.la $(top_builddir)/net/libnginz_net.la $(top_builddir)/apps/load_balancer/libnginz_lb.la
NGINZ_HTTP_INCLUDES=-Iinc -I$(top_builddir)/base/inc -I$(top_builddir)/net/inc  -I$(top_builddir)/apps/load_balancer/inc 


NGINZ_CFLAGS=$(AROOP_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
libnginz_http_la_SOURCES = $(NGINZ_HTTP_SOURCE_FILES)
libnginz_http_la_CPPFLAGS = $(NGINZ_CFLAGS) $(NGINZ_HTTP_INCLUDES)
libnginz_http_la_LDFLAGS = $(NGINZ_HTTP_LDFLAGS)

nginz_http_main_SOURCES = $(NGINZ_HTTP_SOURCE_FILES) src/main.c
#nginz_http_main_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_HTTP_INCLUDES) 
nginz_http_main_CPPFLAGS = $(NGINZ_CFLAGS) -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_HTTP_INCLUDES) 
nginz_http_main_LDFLAGS= -static $(NGINZ_HTTP_LDFLAGS) @AROOP_DEBUG_LIBS@ 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginz_http@PACKAGE_SUFFIX@.pc

libnginz_http@PACKAGE_SUFFIX@.pc: libnginz_http.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
