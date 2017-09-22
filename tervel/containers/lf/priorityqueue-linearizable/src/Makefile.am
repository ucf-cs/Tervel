SUBDIRS = priorityqueue/linden priorityqueue/shavit 

EXTRA_DIST = common/assert.h\
			 common/timehelper.h\
			 common/threadbarrier.h\
			 priorityqueue/mdlist/allocator.h\
			 priorityqueue/mdlist/mdlist.h\
			 priorityqueue/queueadaptor.h


bin_PROGRAMS = next

next_SOURCES = main.cc \
			   common/timehelper.cc\
			   common/assert.cc\
			   priorityqueue/monoheap/monoheap.cc\
			   priorityqueue/vectorheap/vheap.cc\
			   priorityqueue/mdlist/mdlist.cc\
			   priorityqueue/mdlist/allocator.cc


#next_CPPFLAGS = $(WARN_FLAGS) -std=c++11 -mavx
next_CPPFLAGS = $(WARN_FLAGS) -std=c++11\
				-DCACHE_LINE_SIZE=`getconf LEVEL1_DCACHE_LINESIZE`\
				-I$(top_builddir)/src -I$(top_srcdir)/src

next_LDADD = $(BOOST_LDFLAGS) priorityqueue/linden/liblpq.la priorityqueue/shavit/libspq.la
