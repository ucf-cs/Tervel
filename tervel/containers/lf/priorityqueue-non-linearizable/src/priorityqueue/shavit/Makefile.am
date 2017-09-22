EXTRA_DIST = gc.h\
			 intel_defns.h\
			 portable_defns.h\
			 ptst.h\
			 random.h\
			 set.h

noinst_LTLIBRARIES = libspq.la

libspq_la_SOURCES = gc.c\
					ptst.c\
					skip_cas.c


libspq_la_CFLAGS = -O3 -DINTEL -std=c99 $(WARN_FLAGS) \
				   -DCACHE_LINE_SIZE=`getconf LEVEL1_DCACHE_LINESIZE`
					  -I$(top_builddir)/src/priorityqueue/shavit -I$(top_srcdir)/src/priorityqueue/shavit
