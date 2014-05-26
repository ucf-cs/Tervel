CXX      = g++-4.7
CXXFLAGS = -Wall -Wunused-function -g -std=c++11 -march=native -m64 -pthread \
  -fno-strict-aliasing -pg
  #-Wfatal-errors -Werror
CPPFLAGS = -DINTEL -DNO_WAIT_FREE #-DDEBUG=1

INC = -I../
LIB = -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -ltbb# -lllalloc

SOURCES = main_buffer.cc \
  util/progress_assurance.cc util/memory/hp/hazard_pointer.cc \
  util/memory/hp/hp_list.cc util/memory/rc/descriptor_pool.cc \
  util/memory/rc/pool_manager.cc util/info.cc util/thread_context.cc

EXECUTABLE =main

OBJECTS = $(SOURCES:.cc=.o)
CPPFLAGS += $(INC)


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) main
	$(RM) $(OBJECTS)
