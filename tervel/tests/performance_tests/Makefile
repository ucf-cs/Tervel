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
CXXFLAGS =  -DCONTAINER_FILE=$(input) -Wall -Werror  -std=c++11  -march=native -m64 -pthread -fno-strict-aliasing

DEBUG = -DDEBUG=1 -g -g3
# RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG) $(RELEASE)

CPPFLAGS = -DINTEL
LIB = $(cLIB) -ldl -lgflags -lpthread -lrt -lllalloc

EXECUTABLE ?=src/main.cc
OBJECTS = $(SOURCES:.cc=.o)

output ?= $(input).x
OUTPUT = executables/


.PHONY: all
all:  wfringbuffer lfmcasbuffer lock tbb linux tsigas

wfringbuffer:
	$(MAKE) test input="tervel_api/wf_ringbuffer_api.h" output="wfringbuffer.x" SOURCES="$(shell find ../../util/ -name '*.cc')" INC="-I../../../"

lfmcasbuffer:
	$(MAKE) test input="tervel_api/lf_mcasbuffer_api.h" output="lfmcasbuffer.x" SOURCES="$(shell find ../../util/ -name '*.cc')" INC="-I../../../"

tsigas:
	$(MAKE) test input="api/ringbuffer/tsigas.h" output="tsigas.x" INC="-I/Libraries/cds-1.2.0/ " cLIB="-L/Libraries/cds-1.2.0/bin/gcc-amd64-linux-64 -lrt  -lcds"

tbb:
	$(MAKE) test input="api/ringbuffer/tbb.h" output="tbb.x" cLIB="-ltbb"

lock:
	$(MAKE) test input="api/ringbuffer/lock.h" output="lock.x"

linux:
	$(MAKE) test input="api/ringbuffer/linux.h" output="linux.x"

.PHONY: test
test: $(SOURCES) $(EXECUTABLE)

.PHONY: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	mkdir -p $(OUTPUT)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC) -o $(OUTPUT)$(output) $(EXECUTABLE) $^ $(LIB) $(TOBJS)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x

.PHONY: clean-all
clean-all:
	$(RM) $(OUTPUT)*.x
	$(RM) $(shell find ../../util/ -name '*.o')
