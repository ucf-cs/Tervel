CXX      ?= g++-4.7
CXXFLAGS = -Wall -g -std=c++11 -march=native -m64 -pthread -fno-strict-aliasing
CPPFLAGS = -DDEBUG=1 -DINTEL

LIBS    = -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -I../# -lllalloc

CPPFLAGS += $(INC)

SOURCES=main.cc util/progress_assurance.cc util/memory/hp/hazard_pointer.cc util/memory/hp/hp_list.cc util/memory/rc/descriptor_pool.cc util/memory/rc/pool_manager.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=main


all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LIBS)

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LIBS)


.PHONY: clean
clean:
	$(RM) main
	$(RM) *.o
