EXTRA_DIST =

noinst_LTLIBRARIES = libfd.la

libfd_la_SOURCES = skip_cas.c\
				   gc.c\
				   ptst.c


libfd_la_CFLAGS = -O3 -DINTEL -std=c99 $(WARN_FLAGS) \
				  -DCACHE_LINE_SIZE=`getconf LEVEL1_DCACHE_LINESIZE`
				  -I$(top_builddir)/src/dictionary/fraser -I$(top_srcdir)/src/dictionary/fraser
