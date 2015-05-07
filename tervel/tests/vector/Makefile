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
CONTAINERTYPE ?= -DUSE_WF_VECTOR
CXXFLAGS =  $(CONTAINERTYPE) -Wall -Wunused-function -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing

DEBUG = -DDEBUG=1 -g -fpermissive -DDEBUG_POOL=1 #  -DMAX_WAIT_FREE
#RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG) $(RELEASE)

CPPFLAGS = -DINTEL
INC += $(TINC) -I../../../
LIB += $(TLIB) -ldl -lgflags -lpthread -lrt
CPPFLAGS += $(INC)

SOURCES = main.cc $(TSOURCES)
OUTPUT =Executables/

EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: WFTEST_DEBUG


test: $(SOURCES) $(EXECUTABLE)
	rm main.o
	mkdir -p $(OUTPUT)
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
