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
        TSOURCES="$(shell find ../../util/ -name '*.cc')" \
	EXECUTABLE=wfvector.x

WFTEST_DEBUG:
	$(MAKE) test  CONTAINERTYPE=-DUSE_WF_VECTOR \
        TSOURCES="$(shell find ../../util/ -name '*.cc')" \
	EXECUTABLE=wfvector.x

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x  $(shell find ../../util/ -name "*.o")
	$(RM) $(OBJECTS)
