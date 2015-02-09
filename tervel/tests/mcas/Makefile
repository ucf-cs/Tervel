CXX      = g++-4.8
CXXFLAGS = -Wall -Wunused-function -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing

DEBUG = -DDEBUG=1 -p -pg
RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(RELEASE)

CPPFLAGS = -DINTEL -DSET_WAIT_FREE=1 -DSET_HELP_DELAY=1
INC += $(TINC) -I../../../../tervel-library/
LIB += $(TLIB) -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -lllalloc
CPPFLAGS += $(INC)

TSOURCES=../../util/progress_assurance.cc \
        ../../util/memory/hp/hazard_pointer.cc ../../util/memory/hp/hp_list.cc \
        ../../util/memory/rc/descriptor_pool.cc ../../util/thread_context.cc \
        ../../util/memory/rc/pool_manager.cc ../../util/info.cc \
        ../../util/memory/rc/descriptor_read_first_op.cc
SOURCES = main.cc $(TSOURCES)

OUTPUT =Executables/
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: test


test: $(SOURCES) $(EXECUTABLE)
	mkdir -p $(OUTPUT)
	mv $(EXECUTABLE) $(OUTPUT)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x ../../util/progress_assurance.o \
        ../../util/memory/hp/hazard_pointer.o ../../util/memory/hp/hp_list.o \
        ../../util/memory/rc/descriptor_pool.o ../../util/thread_context.o \
        ../../util/memory/rc/pool_manager.o ../../util/info.o \
        ../../util/memory/rc/descriptor_read_first_op.o
	$(RM) $(OBJECTS)
