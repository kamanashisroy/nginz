
TARGET_BIN=nginez_main
TARGET_O=libnginez.o
TARGET_BASIC_O=libnginez_basic.o
#TARGET_HAS_THREAD_O=libnginez_has_thread.o
TARGET_LAR=libnginez.la
TARGET_AR=libnginez.a libnginez_debug.a
bin_PROGRAMS=$(TARGET_BIN) $(TARGET_O) $(TARGET_BASIC_O) $(TARGET_HAS_THREAD_O)
lib_LTLIBRARIES=$(TARGET_LAR)
lib_LIBRARIES=$(TARGET_AR)
NGINEZ_SOURCE_FILES = \
src/fiber.c     src/plugin.c	src/plugin_manager.c	src/scanner.c \
src/event_loop.c \
src/shake.c	src/shake/help.c \
src/parallel/pipeline.c	src/parallel/fork.c 

pkgincludedir=$(includedir)/nginez@PACKAGE_SUFFIX@
nobase_pkginclude_HEADERS= \
inc/event_loop.h \
inc/fiber.h       inc/plugin.h	inc/plugin_manager.h	inc/scanner.h

NGINEZ_INCLUDES=-Iinc
NGINEZ_CFLAGS=$(AROOP_CFLAGS)

libnginez_la_SOURCES = $(NGINEZ_SOURCE_FILES)
libnginez_la_CPPFLAGS = $(NGINEZ_CFLAGS) $(NGINEZ_INCLUDES)
#libnginez_static_a_SOURCES = $(NGINEZ_SOURCE_FILES)
#libnginez_static_a_CPPFLAGS = $(NGINEZ_CFLAGS) $(NGINEZ_INCLUDES)
libnginez_debug_a_SOURCES = $(NGINEZ_SOURCE_FILES)
libnginez_debug_a_CPPFLAGS = $(NGINEZ_CFLAGS) -ggdb3 -DAROOP_OPP_PROFILE -DAROOP_OPP_DEBUG -DMTRACE -DOPP_DEBUG $(NGINEZ_INCLUDES) 
#libnginez_debug_a_CPPFLAGS = -ggdb3 -DAROOP_OPP_PROFILE -DAROOP_OPP_DEBUG -DMTRACE $(NGINEZ_INCLUDES) 
libnginez_o_SOURCES = $(NGINEZ_SOURCE_FILES)
libnginez_o_CPPFLAGS = $(NGINEZ_CFLAGS) $(NGINEZ_INCLUDES)
libnginez_basic_o_SOURCES = $(NGINEZ_SOURCE_FILES)
libnginez_basic_o_CPPFLAGS= $(NGINEZ_CFLAGS)-DAROOP_BASIC $(NGINEZ_INCLUDES)
#libnginez_has_thread_o_SOURCES = $(NGINEZ_SOURCE_FILES)
#libnginez_has_thread_o_CPPFLAGS= $(NGINEZ_CFLAGS)-DHAS_THREAD $(NGINEZ_INCLUDES)
nginez_main_SOURCES= $(NGINEZ_SOURCE_FILES) src/main.c
#nginez_main_CPPFLAGS= $(NGINEZ_CFLAGS) -O2 $(NGINEZ_INCLUDES)
nginez_main_CPPFLAGS= $(NGINEZ_CFLAGS) -ggdb3 -DAROOP_OPP_PROFILE -DAROOP_OPP_DEBUG -DMTRACE -DOPP_DEBUG $(NGINEZ_INCLUDES)
#nginez_main_LDFLAGS= $(AROOP_LIBS)
nginez_main_LDFLAGS= $(AROOP_DEBUG_LIBS)


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginez@PACKAGE_SUFFIX@.pc

libnginez@PACKAGE_SUFFIX@.pc: libnginez.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginez.pc
