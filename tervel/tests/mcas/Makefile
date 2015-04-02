CXX      = g++-4.8
CXXFLAGS = -g -Wall -Wunused-function -std=c++11
            -march=native -m64 -pthread -fno-strict-aliasing

DEBUG = -DDEBUG=1 -p -pg
RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG)

CPPFLAGS = -DINTEL -DSET_WAIT_FREE=1 -DSET_HELP_DELAY=1
INC += $(TINC) -I../../../../tervel-library/
LIB += $(TLIB) -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -lllalloc
CPPFLAGS += $(INC)

TSOURCES := $(shell find ../../util/ -name '*.cc')
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
	$(RM) $(OUTPUT)*.x ../../util/*.o \
        ../../util/memory/hp/*.o ../../util/memory/rc/*.o
	$(RM) $(OBJECTS)
