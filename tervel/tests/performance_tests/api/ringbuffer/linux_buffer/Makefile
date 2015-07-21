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
CXXFLAGS = -Wall -march=native -m64 -fno-strict-aliasing -pthread

#DEBUG = -DDEBUG=1 -g -g3
RELEASE = -O3 -DNDEBUG

CXXFLAGS += $(DEBUG) $(RELEASE) $(DS)

CPPFLAGS = -DINTEL
LIB = -ldl -lllalloc -lpthread -std=c++11 -lrt -Wl,--no-as-needed

EXECUTABLE ?=main_lfrb.cc
OBJECTS = $(SOURCES:.cc=.o)

OUTPUT = executables/


.PHONY: all
all:  tervel linux naive boost

run:
	./$(OUTPUT)/tervel.x
	./$(OUTPUT)/linux.x
	./$(OUTPUT)/boost.x
	./$(OUTPUT)/naive.x

tervel:
	$(MAKE) test output="tervel.x" SOURCES="$(shell find ../../../../../util/ -name '*.cc')" INC="-I../../../../../../" DS="-DUSE_Tervel"

naive:
	$(MAKE) test output="naive.x" DS="-DUSE_Naive"
linux:
	$(MAKE) test output="linux.x" DS="-DUSE_linux"

boost:
	$(MAKE) test output="boost.x" DS="-DUSE_Boost"

.PHONY: test
test: $(SOURCES) $(EXECUTABLE)

.PHONY: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	mkdir -p $(OUTPUT)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC) $(EXECUTABLE) -o $(OUTPUT)$(output)  $^ $(LIB)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC) -c -o $@ $< $(LIB) 


.PHONY: clean
clean:
	$(RM) $(OUTPUT)*.x

.PHONY: clean-all
clean-all:
	$(RM) $(OUTPUT)*.x
	$(RM) $(shell find  ../../../../../util/ -name '*.o')
