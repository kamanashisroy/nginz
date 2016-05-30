TESTS = check_binary_coder check_event_loop
check_PROGRAMS = check_binary_coder check_event_loop


AROOP_CORE_INCLUDES=-I$(top_builddir)/base/inc
OPP_FACTORY_LIBS=$(top_builddir)/base/libnginz_base.la

# check binary coder
check_binary_coder_SOURCES = check_binary_coder.c
check_binary_coder_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES) @AROOP_CFLAGS@
check_binary_coder_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@ @AROOP_LIBS@

# check event loop
check_event_loop_SOURCES = check_event_loop.c
check_event_loop_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES) @AROOP_CFLAGS@
check_event_loop_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@ @AROOP_LIBS@
