GCC = g++-4.7
CXXFLAGS = -std=c++0x  -mcx16  -m64  -Wall
LIBS = -I /usr/include/boost  -lpthread -lllalloc

all: 
	$(GCC) -DDEBUG=1 -DDEF_PB_E=1 -g  -DUSE_WF_VECTOR=1 $(CXXFLAGS) tester.cpp -o test.o $(LIBS)

OPT: 
	$(GCC) -DNDEBUG=1 -DDEF_PB_LOCK=1 --O3 -DUSE_WF_VECTOR=1 $(CXXFLAGS) tester.cpp -o test.o $(LIBS)
clean:
	rm *.x	
