
#CPPSRCS		= main_tester_v2.c
#DIRECTORY	= test_executables/eachThread/

CPPSRCS         = main_tester.cpp

CPP		= g++-4.7
CPPFLAGS	= -Wall -std=c++11   -march=native -m64 -pthread -DINTEL -fno-strict-aliasing

#-march=native -fomit-frame-pointer

LFLAGS =   -ldl -lpthread  -I/Libraries/boost_1_51_0/BOOST_INSTAL/include -L/Libraries/boost_1_51_0/BOOST_INSTAL/lib -lboost_system -lboost_regex -lrt -lllalloc -lrt

all: clean mcas

clean:
	rm -f mcas
mcas:
	$(CPP)  -g -ggdb -DDEBUG=1   $(CPPFLAGS) main_tester.cpp -o mcas $(LFLAGS)

