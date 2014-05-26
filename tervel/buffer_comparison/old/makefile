CXX = g++-4.7
CFLAGS = -Wall -g -c -std=c++11
LFLAGS = -Wall -pthread
OBJS = main.o wf_ring_buffer.o

main: $(OBJS)
	$(CXX) $(LFLAGS) $(OBJS) -o main

main.o: main.cpp wf_ring_buffer.h node.h
	$(CXX) $(CFLAGS) main.cpp

wf_ring_buffer.o: wf_ring_buffer.cpp node.h
	$(CXX) $(CFLAGS) wf_ring_buffer.cpp

clean:
	\rm *.o *~ main

tar:
	tar cfv wf_ring_buffer.tar node.h node.cpp wf_ring_buffer.cpp wf_ring_buffer.h main.cpp
