CXX      = g++-4.7
BUFFERTYPE ?= -DUSE_WF_BUFFER
CXXFLAGS =  $(BUFFERTYPE) -Wall -Wunused-function -g -pg -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing -pg

CPPFLAGS = -DINTEL -DNO_WAIT_FREE #-DDEBUG=1
INC = -I../../../
LIB = -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt #-lllalloc
CPPFLAGS += $(INC)

SOURCES = main_buffer.cc $(TSOURCES)
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: WFBuffer

cleanAll: WFBufferClean

test: $(SOURCES) $(EXECUTABLE)

WFBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_WF_BUFFER \
	TSOURCES="../../util/progress_assurance.cc \
	../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
	../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
	../../util/memory/rc/pool_manager.cc ../../util/info.cc" \
	EXECUTABLE=wfbuffer

WFBufferClean:
	$(MAKE) clean \ TSOURCES="../../util/progress_assurance.cc \
	../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
	../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
	../../util/memory/rc/pool_manager.cc ../../util/info.cc" \
	EXECUTABLE=wfbuffer

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) EXECUTABLE
	$(RM) $(OBJECTS)
