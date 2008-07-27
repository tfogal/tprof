lib_LTLIBRARIES = libtprof.la

libtprof_la_SOURCES = tprof.c

# tprof is entirely based around gcc's instrumentation, and is thus completely
# useless without the GNU toolchain.  So `gnu99' C here isn't really all that
# bad.
libtprof_la_CFLAGS = -std=gnu99 @GLIB_CFLAGS@
libtprof_la_LIBADD = @GLIB_LIBS@ -ldl

bin_PROGRAMS = recursive basic

DFLT_CF =-std=c99 -finstrument-functions -fPIC

basic_SOURCES = test.c
basic_CFLAGS = $(DFLT_CF)
basic_LDADD = -ltprof
recursive_SOURCES = recurse.c
recursive_CFLAGS = $(DFLT_CF) @GLIB_CFLAGS@
recursive_LDADD = @GLIB_LIBS@ -ltprof
