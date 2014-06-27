CXX      = g++-4.7
CONTAINERTYPE ?= -DUSE_WF_HASHMAP
CXXFLAGS =  $(CONTAINERTYPE) -Wall -Wunused-function -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing

DEBUG = -DDEBUG=1 -p -pg
RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG)

CPPFLAGS = -DINTEL -DNO_WAIT_FREE #-DDEBUG=1
INC += $(TINC) -I../../../
LIB += $(TLIB) -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -lllalloc
CPPFLAGS += $(INC)

SOURCES = main.cc $(TSOURCES)
OUTPUT =Executables/
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: WFTEST_DEBUG


test: $(SOURCES) $(EXECUTABLE)
	mkdir -p $(OUTPUT)
	rm main.o
	mv $(EXECUTABLE) $(OUTPUT)

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
