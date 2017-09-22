EXTRA_DIST =

noinst_LTLIBRARIES = libdick.la

libdick_la_SOURCES = background.c\
				   garbagecoll.c\
				   intset.c\
				   nohotspot_ops.c\
				   ptst.c\
				   skiplist.c


libdick_la_CFLAGS = -O3 -DNDEBUG -DINTEL -std=gnu99 $(WARN_FLAGS)\
					  -I$(top_builddir)/src/common/dick/atomic_ops -I$(top_srcdir)/src/common/dick/atomic_ops
