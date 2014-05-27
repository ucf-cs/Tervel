CXX      = g++-4.7
BUFFERTYPE ?= -DUSE_WF_BUFFER
CXXFLAGS =  $(BUFFERTYPE) -Wall -Wunused-function -g -pg -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing -pg

CPPFLAGS = -DINTEL -DNO_WAIT_FREE #-DDEBUG=1
INC += $(TINC) -I../../../
LIB += $(TLIB) -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -lllalloc
CPPFLAGS += $(INC)

SOURCES = main_buffer.cc $(TSOURCES)
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: WFBuffer FAATest LockBuffer TBBBuffer TsigasBuffer #MCASBuffer


test: $(SOURCES) $(EXECUTABLE)
	rm main_buffer.o

WFBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_WF_BUFFER \
	TSOURCES="../../util/progress_assurance.cc \
	../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
	../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
	../../util/memory/rc/pool_manager.cc ../../util/info.cc" \
	EXECUTABLE=wfbuffer

FAATest:
	$(MAKE) test BUFFERTYPE=-DUSE_ATOMIC_ONLY \
	EXECUTABLE=faatest

LockBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_COARSE_LOCK \
	EXECUTABLE=lockbuffer

MCASBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_MCAS_BUFFER \
	EXECUTABLE=mcasbuffer

TBBBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_TBB_QUEUE \
	EXECUTABLE=tbbbuffer \
	TLIB=-ltbb

TsigasBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_TSIGAS_QUEUE \
	EXECUTABLE=tsigasbuffer \
	TLIB=-lcds TINC=-I/Libraries/cds-1.2.0/


LinuxBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_LINUX_BUFFER \
	EXECUTABLE=linuxbuffer

LinuxBufferRecreate:
	$(MAKE) test BUFFERTYPE=-DUSE_LINUX_BUFFER_RECREATE \
	EXECUTABLE=linuxbufferrecreate

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) wfbuffer faatest lockbuffer mcasbuffer tbbbuffer tsigasbuffer \
	linuxbuffer linuxbufferrecreate ../../util/progress_assurance.o \
	../../util/memory/hp/hazard_pointer.o ../../util/memory/hp/hp_list.o \
	../../util/memory/rc/descriptor_pool.o ../../util/thread_context.o \
	../../util/memory/rc/pool_manager.o ../../util/info.o
	$(RM) $(OBJECTS)
