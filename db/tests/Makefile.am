TESTS = check_db
check_PROGRAMS = check_db


AROOP_CORE_INCLUDES=-I$(top_builddir)/base/inc -I$(top_builddir)/db/inc
OPP_FACTORY_LIBS=$(top_builddir)/base/libnginz_base.la $(top_builddir)/db/libnginz_db.la 

# check db
check_db_SOURCES = check_db.c
check_db_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES) @AROOP_CFLAGS@
check_db_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@ @AROOP_LIBS@

