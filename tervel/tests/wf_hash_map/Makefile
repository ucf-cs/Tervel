CXX      = g++-4.8
CONTAINERTYPE ?= -DUSE_WF_HASHMAP
CXXFLAGS =  $(CONTAINERTYPE) -Wall -Wunused-function -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing

DEBUG = -DDEBUG=1 -p -pg -g -DMAX_WAIT_FREE=0
RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG)

CPPFLAGS = -DINTEL
INC += $(TINC) -I../../../
LIB += $(TLIB) -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -lllalloc
CPPFLAGS += $(INC) $(TINC)

SOURCES = main.cc $(TSOURCES)
OUTPUT =Executables/
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: WFTEST_DEBUG TBB BOOST STL SPLIT MICHAEL


test: $(SOURCES) $(EXECUTABLE)
	mkdir -p $(OUTPUT)
	rm main.o
	mv $(EXECUTABLE) $(OUTPUT)

TBB:
	$(MAKE) test CONTAINERTYPE=-DUSE_TBB_HASHMAP \
	TLIB="-ltbb" \
	EXECUTABLE=tbbmap.x

BOOST:
	$(MAKE) test CONTAINERTYPE=-DUSE_BOOST_HASHMAP \
	TLIB=" -I/Libraries/boost_1_51_0/BOOST_INSTAL/include -L/Libraries/boost_1_51_0/BOOST_INSTAL/lib -lboost_system -lboost_regex -lrt " \
	EXECUTABLE=tbbmap.x

STL:
	$(MAKE) test CONTAINERTYPE=-DUSE_STL_HASHMAP \
	TLIB=" -I/Libraries/boost_1_51_0/BOOST_INSTAL/include -L/Libraries/boost_1_51_0/BOOST_INSTAL/lib -lboost_system -lboost_regex -lrt " \
	EXECUTABLE=stlmap.x

SPLIT:
	export LD_LIBRARY_PATH=/Libraries/cds-1.2.0/bin/gcc-amd64-linux-64
	$(MAKE) test CONTAINERTYPE="-DUSE_SPLIT_HASHMAP -Dtest_splitlist=2"  \
	TLIB="-I/Libraries/boost_1_51_0/BOOST_INSTAL/include -L/Libraries/boost_1_51_0/BOOST_INSTAL/lib -lboost_system -I/Libraries/cds-1.2.0/ -L/Libraries//bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=split2.x

	$(MAKE) test CONTAINERTYPE="-DUSE_SPLIT_HASHMAP -Dtest_splitlist=3"  \
	TLIB="-I/Libraries/boost_1_51_0/BOOST_INSTAL/include -L/Libraries/boost_1_51_0/BOOST_INSTAL/lib -lboost_system -I/Libraries/cds-1.2.0/ -L/Libraries//bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=split3.x

MICHAEL:
	export LD_LIBRARY_PATH=/Libraries/cds-1.2.0/bin/gcc-amd64-linux-64

	$(MAKE) test CONTAINERTYPE="-DUSE_MICHAEL_HASHMAP -Dtest_michael_map=2" \
	TLIB="-I/Libraries/boost_1_51_0/BOOST_INSTAL/include -L/Libraries/boost_1_51_0/BOOST_INSTAL/lib -lboost_system -I/Libraries/cds-1.2.0/ -L/Libraries//bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=michael2.x

	$(MAKE) test CONTAINERTYPE="-DUSE_MICHAEL_HASHMAP -Dtest_michael_map=3" \
	TLIB="-I/Libraries/boost_1_51_0/BOOST_INSTAL/include -L/Libraries/boost_1_51_0/BOOST_INSTAL/lib -lboost_system -I/Libraries/cds-1.2.0/ -L/Libraries//bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=michael3.x

WFTEST:
	$(MAKE) test CONTAINERTYPE=-DUSE_WF_HASHMAP \
	TSOURCES="../../util/progress_assurance.cc \
	../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
	../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
	../../util/memory/rc/pool_manager.cc ../../util/info.cc" \
	EXECUTABLE=wfhashmap.x

WFTEST_DEBUG:
	$(MAKE) test CONTAINERTYPE=-DUSE_WF_HASHMAP \
	TSOURCES="../../util/progress_assurance.cc \
	../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
	../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
	../../util/memory/rc/pool_manager.cc ../../util/info.cc" \
	EXECUTABLE=wfhashmap.x

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x ../../util/progress_assurance.o \
	../../util/memory/hp/hazard_pointer.o ../../util/memory/hp/hp_list.o \
	../../util/memory/rc/descriptor_pool.o ../../util/thread_context.o \
	../../util/memory/rc/pool_manager.o ../../util/info.o
	$(RM) $(OBJECTS)
