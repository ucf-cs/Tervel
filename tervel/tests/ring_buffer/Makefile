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



CXX      = g++-4.8
BUFFERTYPE ?= -DUSE_WF_BUFFER
CXXFLAGS =  $(BUFFERTYPE) -Wall -Wunused-function -std=c++11 \
            -march=native -m64 -pthread -fno-strict-aliasing

DEBUG = -DDEBUG=1 -g -pg
RELEASE = -O3 -DNDEBUG

#CXXFLAGS += $(DEBUG)

CPPFLAGS = -DINTEL -DNO_WAIT_FREE #-DDEBUG=1
INC += $(TINC) -I../../../
LIB += $(TLIB) -ldl -lgflags -lpthread -lboost_system -lboost_regex -lrt -lllalloc
CPPFLAGS += $(INC)

SOURCES = main_buffer.cc $(TSOURCES)
OUTPUT = Executables/
EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)


all: WFBuffer FAATest LockBuffer TBBBuffer TsigasBuffer LinuxBuffer MCASBuffer


test: $(SOURCES) $(EXECUTABLE)
	mv $(EXECUTABLE) $(OUTPUT)
	rm main_buffer.o

WFBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_WF_BUFFER \
        TSOURCES="$(shell find ../../util/ -name '*.cc')" \
	EXECUTABLE=wfbuffer.x

FAATest:
	$(MAKE) test BUFFERTYPE=-DUSE_ATOMIC_ONLY \
	EXECUTABLE=faatest.x

LockBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_COARSE_LOCK \
	EXECUTABLE=lockbuffer.x

MCASBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_MCAS_BUFFER \
        TSOURCES="$(shell find ../../util/ -name '*.cc')" \
	EXECUTABLE=mcasbuffer.x

TBBBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_TBB_QUEUE \
	EXECUTABLE=tbbbuffer.x \
	TLIB=-ltbb

TsigasBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_TSIGAS_QUEUE \
	EXECUTABLE=tsigasbuffer.x \
	TLIB=-lcds TINC=-I/Libraries/cds-1.2.0/


LinuxBuffer:
	$(MAKE) test BUFFERTYPE=-DUSE_LINUX_BUFFER \
	EXECUTABLE=linuxbuffer.x

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x $(shell find ../../util/ -name '*.o')
	$(RM) $(OBJECTS)
