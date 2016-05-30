TESTS = check_binary_coder check_event_loop check_scanner check_single_fiber check_pipeline
check_PROGRAMS = check_binary_coder check_event_loop check_scanner check_single_fiber check_pipeline


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

# check scanner
check_scanner_SOURCES = check_scanner.c
check_scanner_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES) @AROOP_CFLAGS@
check_scanner_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@ @AROOP_LIBS@

# check single fiber
check_single_fiber_SOURCES = check_single_fiber.c
check_single_fiber_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES) @AROOP_CFLAGS@
check_single_fiber_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@ @AROOP_LIBS@

# check pipeline
check_pipeline_SOURCES = check_pipeline.c
check_pipeline_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES) @AROOP_CFLAGS@
check_pipeline_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@ @AROOP_LIBS@

