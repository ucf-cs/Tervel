EXTRA_DIST =

noinst_LTLIBRARIES = libhd.la

libhd_la_SOURCES = optimistic.c\
				   skiplist-lock.c


libhd_la_CFLAGS = -O3 -DNDEBUG -DINTEL -std=gnu99 $(WARN_FLAGS)\
					  -I$(top_builddir)/src/common/dick/atomic_ops -I$(top_srcdir)/src/common/dick/atomic_ops
