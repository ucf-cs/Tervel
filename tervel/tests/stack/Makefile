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
CXXFLAGS =  -DCONTAINER_FILE=$(input) -Wall -Werror  -std=c++11  -march=native -m64 -pthread -fno-strict-aliasing -DTERVEL_PROG_ASSUR_ALWAYS_ANNOUNCE -DTERVEL_PROG_ASSUR_ALWAYS_CHECK

DEBUG = -DDEBUG=1 -g 
# RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG) $(RELEASE)

CPPFLAGS = -DINTEL
INC= -I../../../
LIB += -ldl -lgflags -lpthread -lrt

SOURCES = main.cc $(shell find ../../util/ -name '*.cc')

EXECUTABLE ?=main
OBJECTS = $(SOURCES:.cc=.o)

output ?= $(input).x
OUTPUT = Executables/

all: WFTEST LFTEST

test: $(SOURCES) $(EXECUTABLE)
	mkdir -p $(OUTPUT)

LFTEST:
	$(MAKE) test input="api/lf_stack_api.h" output="lf_stack.x"
	

WFTEST:
	$(MAKE) test input="api/wf_stack_api.h" output="wf_stack.x"


$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC)  -o $(OUTPUT)$(output) $^ $(LIB) $(TOBJS)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x
	$(RM) $(OBJECTS)
