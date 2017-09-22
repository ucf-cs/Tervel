EXTRA_DIST =

noinst_LTLIBRARIES = libcitrus.la

libcitrus_la_SOURCES = citrus.c\
					   new_urcu.c


libcitrus_la_CFLAGS = -O3 -DNDEBUG -DINTEL -std=gnu99 $(WARN_FLAGS)\
					  -I$(top_builddir)/src/common/dick/atomic_ops -I$(top_srcdir)/src/common/dick/atomic_ops
