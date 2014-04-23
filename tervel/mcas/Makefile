CXX      ?= g++-4.7
CXXFLAGS = -Wall -g -std=c++11 -march=native -m64 -pthread -fno-strict-aliasing
CPPFLAGS = -DDEBUG=1 -DINTEL

INC     = -I/Libraries/boost_1_51_0/BOOST_INSTAL/include
LDFLAGS = -L/Libraries/boost_1_51_0/BOOST_INSTAL/lib
LIBS    = -ldl -lpthread -lboost_system -lboost_regex -lrt # -lllalloc

CPPFLAGS += $(INC)

.PHONY: all
all: mcas

mcas: main_tester.o ucf_threading.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LIBS)

main_tester.o: main_tester.cc wf_mcas_3.h
ucf_threading.o: ucf_threading.cc ucf_threading.h

.PHONY: clean
clean:
	$(RM) mcas
	$(RM) *.o
