#The MIT License (MIT)
#
#Copyright (c) 2015 University of Central Florida's Computer Software Engineering
#Scalable & Secure Systems (CSE - S3) Lab
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.
#

CXX      = g++-4.8
CONTAINERTYPE ?= -DUSE_WF_HASHMAP
CXXFLAGS =  $(CONTAINERTYPE)  -std=c++11  -march=native -m64 -pthread -fno-strict-aliasing $(TFLAGS)

DEBUG = -DDEBUG=1 -g 
# RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG) $(RELEASE)

CPPFLAGS = -DINTEL
INC += $(TINC)
LIB += $(TLIB) -ldl -lgflags -lpthread  -lrt


SOURCES = main.cc $(TSOURCES)
OUTPUT =Executables/
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


#all: CLIFF WFTEST TBB BOOST STL SPLIT MICHAEL WFOLD

all: WFTEST  WFTESTND

test: $(SOURCES) $(EXECUTABLE)
	mkdir -p $(OUTPUT)
	rm main.o
	mv $(EXECUTABLE) $(OUTPUT)

TBB:
	$(MAKE) test CONTAINERTYPE=-DUSE_TBB_HASHMAP \
	TLIB="-ltbb" \
	EXECUTABLE=tbbmap.x

BOOST:
	$(MAKE) test CONTAINERTYPE=-DUSE_BOOST_HASHMAP \
	EXECUTABLE=boostmap.x

STL:
	$(MAKE) test CONTAINERTYPE=-DUSE_STL_HASHMAP \
	EXECUTABLE=stlmap.x

SPLIT:
	export LD_LIBRARY_PATH=/Libraries/cds-1.2.0/bin/gcc-amd64-linux-64
	$(MAKE) test CONTAINERTYPE="-DUSE_SPLIT_HASHMAP -Dtest_splitlist=2"  \
	TLIB="-L/Libraries/bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=split2.x

	$(MAKE) test CONTAINERTYPE="-DUSE_SPLIT_HASHMAP -Dtest_splitlist=3"  \
	TLIB="-L/Libraries/bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=split3.x

MICHAEL:
	export LD_LIBRARY_PATH=/Libraries/cds-1.2.0/bin/gcc-amd64-linux-64

	$(MAKE) test CONTAINERTYPE="-DUSE_MICHAEL_HASHMAP -Dtest_michael_map=2" \
	TLIB="-L/Libraries/bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=michael2.x

	$(MAKE) test CONTAINERTYPE="-DUSE_MICHAEL_HASHMAP -Dtest_michael_map=3" \
	TLIB="-L/Libraries/bin/gcc-amd64-linux-64 -lrt  -lcds -lrt  -fpermissive" \
	TINC="-I/Libraries/cds-1.2.0/  " \
	EXECUTABLE=michael3.x

WFTEST:
	$(MAKE) WFTEST_SUB EXPAN_RATE="6"
	$(MAKE) WFTEST_SUB EXPAN_RATE="4"

WFTEST_SUB:
	$(MAKE) test CONTAINERTYPE="-DUSE_WF_HASHMAP -DHASHMAP_EXPANSION_RATE=$(EXPAN_RATE)" \
	TINC="-I../../../../tervel-library/" \
        TSOURCES="$(shell find ../../util/ -name '*.cc')" \
	EXECUTABLE=wfhashmap_$(EXPAN_RATE).x

WFTESTND:
	$(MAKE) WFTESTND_SUB EXPAN_RATE="6"
	$(MAKE) WFTESTND_SUB EXPAN_RATE="4"

WFTESTND_SUB:
	$(MAKE) test CONTAINERTYPE="-DUSE_WF_HASHMAP_ND -DHASHMAP_EXPANSION_RATE=$(EXPAN_RATE)" \
	TINC="-I../../../../tervel-library/" \
        TSOURCES="$(shell find ../../util/ -name '*.cc')" \
	EXECUTABLE=wfhashmap_nd_$(EXPAN_RATE).x


$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC)  -o $@ $^ $(LIB) $(TOBJS)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)  $(INC) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x $(shell find ../../util/ -name "*.o")
	$(RM) $(OBJECTS)
