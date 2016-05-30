
TARGET_LAR=libnginz_chat.la
TARGET_AR=libnginz_chat.a libnginz_chat_debug.a
lib_LIBRARIES=$(TARGET_AR)
NGINZ_NET_SOURCE_FILES = \
src/apps/web_chat/web_chat.c	src/apps/web_chat/page_chat.c	src/apps/web_chat/web_session_factory.c

NGINZ_NET_INCLUDES=-Iinc -I../core/inc
NGINZ_CFLAGS=$(AROOP_CFLAGS) $(MEMCACHED_CFLAGS) -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
libnginz_chat_debug_a_SOURCES = $(NGINZ_NET_SOURCE_FILES)
libnginz_chat_debug_a_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_NET_INCLUDES) 
libnginz_chat_a_SOURCES = $(NGINZ_NET_SOURCE_FILES)
libnginz_chat_a_CPPFLAGS = $(NGINZ_CFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINZ_NET_INCLUDES) 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnginz_chat@PACKAGE_SUFFIX@.pc

libnginz_chat@PACKAGE_SUFFIX@.pc: libnginz_chat.pc
	cp $< $@

#MAINTAINERCLEANFILES = nginz.pc
