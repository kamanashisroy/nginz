
TARGET_LAR=libnginz_lb.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINZ_LOAD_BALANCER_SOURCE_FILES = \
src/load_balancer_subsystem.c

NGINZ_BASE_LDFLAGS=$(top_builddir)/base/libnginz_base.la
NGINZ_NET_LDFLAGS=$(top_builddir)/net/libnginz_net.la
NGINZ_LOAD_BALANCER_INCLUDES=-Iinc -I$(top_builddir)/base/inc -I$(top_builddir)/net/inc


NGINZ_CFLAGS=$(AROOP_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
libnginz_lb_la_SOURCES = $(NGINZ_LOAD_BALANCER_SOURCE_FILES)
libnginz_lb_la_CPPFLAGS = $(NGINZ_CFLAGS) $(NGINZ_LOAD_BALANCER_INCLUDES)
libnginz_lb_la_LDFLAGS = $(NGINZ_BASE_LDFLAGS) $(NGINZ_NET_LDFLAGS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginz_lb@PACKAGE_SUFFIX@.pc

libnginz_lb@PACKAGE_SUFFIX@.pc: libnginz_lb.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
