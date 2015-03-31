CXX      = g++-4.8
CONTAINERTYPE ?= -DUSE_WF_VECTOR
CXXFLAGS =  $(CONTAINERTYPE) -Wall -Wunused-function -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing

DEBUG = -DDEBUG=1 -p -pg -g -fpermissive -DDEBUG_POOL=1 #  -DMAX_WAIT_FREE
#RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG) $(RELEASE)

CPPFLAGS = -DINTEL
INC += $(TINC) -I../../../
LIB += $(TLIB) -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -lllalloc
CPPFLAGS += $(INC)

SOURCES = main.cc $(TSOURCES)
OUTPUT =Executables/
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: WFTEST_DEBUG


test: $(SOURCES) $(EXECUTABLE)
	rm main.o
	mv $(EXECUTABLE) $(OUTPUT)

WFTEST:
	$(MAKE) test CONTAINERTYPE=-DUSE_WF_VECTOR \
	TSOURCES="../../util/progress_assurance.cc \
	../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
	../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
	../../util/memory/rc/pool_manager.cc ../../util/info.cc" \
	EXECUTABLE=wfvector.x

WFTEST_DEBUG:
	$(MAKE) test  CONTAINERTYPE=-DUSE_WF_VECTOR \
	TSOURCES="../../util/progress_assurance.cc \
	../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
	../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
	../../util/memory/rc/pool_manager.cc ../../util/info.cc ../../util/memory/hp/list_manager.cc" \
	EXECUTABLE=wfvector.x

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x ../../util/*.o \
	../../util/memory/*.o \
	../../util/memory/rc/*.o  \
	../../util/memory/rc/*.o 
	$(RM) $(OBJECTS)
