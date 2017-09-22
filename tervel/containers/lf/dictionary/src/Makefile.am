SUBDIRS = dictionary/fraser dictionary/dick dictionary/herlihyoptimistic dictionary/citrus

EXTRA_DIST =

bin_PROGRAMS = cpdict

cpdict_SOURCES = main.cc \
			   common/timehelper.cc\
			   common/assert.cc\
			   common/wicht/HazardManager.cpp\
			   dictionary/mdlist/mdlist.cc\
			   dictionary/mdlist/allocator.cc


#cpdict_CPPFLAGS = $(WARN_FLAGS) -std=c++11 -mavx
cpdict_CPPFLAGS = $(WARN_FLAGS) -std=c++11\
				-DCACHE_LINE_SIZE=`getconf LEVEL1_DCACHE_LINESIZE`\
				-I$(top_builddir)/src -I$(top_srcdir)/src\
				-I$(top_builddir)/src/common/dick/atomic_ops -I$(top_srcdir)/src/common/dick/atomic_ops

cpdict_LDADD = $(BOOST_LDFLAGS) dictionary/fraser/libfd.la dictionary/dick/libdick.la dictionary/herlihyoptimistic/libhd.la dictionary/citrus/libcitrus.la
