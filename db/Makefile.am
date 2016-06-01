
#TARGET_BIN=nginz_db_main
TARGET_LAR=libnginz_db.la
#bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINZ_DB_SOURCE_FILES = \
src/db_subsystem.c \
src/async_db.c src/async_db_master.c \
src/db.c

NGINZ_BASE_LDFLAGS=$(top_builddir)/base/libnginz_base.la
NGINZ_DB_INCLUDES=-Iinc -I$(top_builddir)/base/inc

NGINZ_CFLAGS=$(AROOP_CFLAGS) $(MEMCACHED_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
libnginz_db_la_SOURCES = $(NGINZ_DB_SOURCE_FILES)
libnginz_db_la_CPPFLAGS = $(NGINZ_CFLAGS) $(NGINZ_DB_INCLUDES) 
libnginz_db_la_LDFLAGS = $(NGINZ_BASE_LDFLAGS)

#nginz_db_main_SOURCES = $(NGINZ_DB_SOURCE_FILES) src/main.c
#nginz_db_main_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_DB_INCLUDES) 
#nginz_db_main_LDFLAGS= -static $(MEMCACHED_LIBS) $(NGINZ_BASE_LDFLAGS) @AROOP_DEBUG_LIBS@ 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginz_db@PACKAGE_SUFFIX@.pc

libnginz_db@PACKAGE_SUFFIX@.pc: libnginz_db.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
