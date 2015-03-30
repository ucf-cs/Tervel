CXX      = g++-4.8
CONTAINERTYPE ?= -DUSE_WF_HASHMAP
CXXFLAGS =  $(CONTAINERTYPE)  -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing $(TFLAGS)

# DEBUG = -DDEBUG=1 -p -pg -g -DMAX_WAIT_FREE=0
RELEASE = -O3 -DNDEBUG -DSET_WAIT_FREE=10000 -DSET_HELP_DELAY=100000

CXXFLAGS += $(DEBUG)

CPPFLAGS = -DINTEL
INC += $(TINC)
LIB += $(TLIB) -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -lllalloc


SOURCES = main.cc $(TSOURCES)
OUTPUT =Executables/
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: CLIFF WFTEST TBB BOOST STL SPLIT MICHAEL WFOLD


test: $(SOURCES) $(EXECUTABLE)
	mkdir -p $(OUTPUT)
	rm main.o
	mv $(EXECUTABLE) $(OUTPUT)

CLIFF:
	$(MAKE) test CONTAINERTYPE=-DUSE_CLIFF_HASHMAP \
	TOBJS="/Libraries/nbds.0.4.3/map.o /Libraries/nbds.0.4.3/hashtable.o /Libraries/nbds.0.4.3/runtime.o /Libraries/nbds.0.4.3/rcu.o /Libraries/nbds.0.4.3/lwt.o /Libraries/nbds.0.4.3/mem.o /Libraries/nbds.0.4.3/nstring.o /Libraries/nbds.0.4.3/list.o /Libraries/nbds.0.4.3/skiplist.o" \
	TINC="-I/Libraries/nbds.0.4.3/include/" \
	EXECUTABLE=cliff.x

TBB:
	$(MAKE) test CONTAINERTYPE=-DUSE_TBB_HASHMAP \
	TLIB="-ltbb" \
	EXECUTABLE=tbbmap.x

BOOST:
	$(MAKE) test CONTAINERTYPE=-DUSE_BOOST_HASHMAP \
	EXECUTABLE=boostmap.x

STL:
	$(MAKE) test CONTAINERTYPE=-DUSE_STL_HASHMAP \
	EXECUTABLE=stlmap.x

SPLIT:
	export LD_LIBRARY_PATH=/Libraries/cds-1.2.0/bin/gcc-amd64-linux-64
	$(MAKE) test CONTAINERTYPE="-DUSE_SPLIT_HASHMAP -Dtest_splitlist=2"  \
	TLIB="-L/Libraries/bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=split2.x

	$(MAKE) test CONTAINERTYPE="-DUSE_SPLIT_HASHMAP -Dtest_splitlist=3"  \
	TLIB="-L/Libraries/bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=split3.x

MICHAEL:
	export LD_LIBRARY_PATH=/Libraries/cds-1.2.0/bin/gcc-amd64-linux-64

	$(MAKE) test CONTAINERTYPE="-DUSE_MICHAEL_HASHMAP -Dtest_michael_map=2" \
	TLIB="-L/Libraries/bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=michael2.x

	$(MAKE) test CONTAINERTYPE="-DUSE_MICHAEL_HASHMAP -Dtest_michael_map=3" \
	TLIB="-L/Libraries/bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=michael3.x

WFTEST:
	$(MAKE) WFTEST_SUB EXPAN_RATE="6"
	$(MAKE) WFTEST_SUB EXPAN_RATE="4"
WFTEST_SUB:
	$(MAKE) test CONTAINERTYPE="-DUSE_WF_HASHMAP -DHASHMAP_EXPANSION_RATE=$(EXPAN_RATE)" \
	TINC="-I../../../  " \
	TSOURCES="../../util/progress_assurance.cc \
	../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
	../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
	../../util/memory/rc/pool_manager.cc ../../util/info.cc" \
	EXECUTABLE=wfhashmap_$(EXPAN_RATE).x

WFOLD:
	$(MAKE) WFOLD_SUB EXPAN_RATE="4"
	$(MAKE) WFOLD_SUB EXPAN_RATE="6"
WFOLD_SUB:
	$(MAKE) test CONTAINERTYPE="-DUSE_WF_OLD_HASHMAP -DSUB_POW=$(EXPAN_RATE)" \
	EXECUTABLE=wf_old_hashmap_$(EXPAN_RATE).x \
	TSOURCES="/home/damian/archive/WFHashTable/WaitFreeHashTable.cpp"


$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC)  -o $@ $^ $(LIB) $(TOBJS)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)  $(INC) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x ../../util/progress_assurance.o \
	../../util/memory/hp/hazard_pointer.o ../../util/memory/hp/hp_list.o \
	../../util/memory/rc/descriptor_pool.o ../../util/thread_context.o \
	../../util/memory/rc/pool_manager.o ../../util/info.o
	$(RM) $(OBJECTS)
