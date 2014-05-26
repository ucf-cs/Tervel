CC = g++-4.7 -g
OUTDIR = Executables_v2
CXXFLAGS = -std=c++11 -mcx16  -m64  
LIBFLAGS = -pthread -lllalloc -lrt -ltbb -lcds
OFLAGS = -fno-strict-aliasing #-std=c++0x  #-O3 -DNDEBUG 
LDFLAGS = -pg -L/Libraries/cds-1.2.0/bin/gcc-amd64-linux-64 -lrt -I/Libraries/cds-1.2.0/ -lrt
INCLUDES = -I/home/andrew/lib/boost_1_55_0

all: dir WFRingBuffer PaddedWFRingBuffer CoarseLockBuffer OnlyAtomicFAA McasBuffer TbbQueue TsigasQueue LinuxBuffer
ready: CoarseLockBuffer OnlyAtomicFAA McasBuffer TbbQueue

dir:
	mkdir -p $(OUTDIR)

WFRingBuffer: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_WF_BUFFER=1 $(CXXFLAGS) $(INCLUDES) tester_v2.cpp wf_ring_buffer/wf_ring_buffer.cpp wf_ring_buffer/wf_macros.cpp -o $(OUTDIR)/wf_tester_v2.x $(LIBFLAGS)

PaddedWFRingBuffer: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_PADDED_WF_BUFFER=1 $(CXXFLAGS) $(INCLUDES) tester_v2.cpp wf_ring_buffer_padded/wf_ring_buffer.cpp wf_ring_buffer/wf_macros.cpp -o $(OUTDIR)/padded_wf_tester_v2.x $(LIBFLAGS)

CoarseLockBuffer: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_COARSE_LOCK=1 $(CXXFLAGS) $(INCLUDES) tester_v2.cpp coarse_lock_buffer/coarse_lock_buffer.cpp -o $(OUTDIR)/coarse_lock_tester_v2.x $(LIBFLAGS)
   
OnlyAtomicFAA: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_ATOMIC_ONLY=1 $(CXXFLAGS) $(INCLUDES) tester_v2.cpp only_atomic_FAA/only_atomic_FAA.cpp -o $(OUTDIR)/atomics_tester_v2.x $(LIBFLAGS)

McasBuffer: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_MCAS_BUFFER=1 $(CXXFLAGS) $(INCLUDES) tester_v2.cpp -o $(OUTDIR)/mcas_tester_v2.x $(LIBFLAGS)

TbbQueue: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_TBB_QUEUE=1 $(CXXFLAGS) $(INCLUDES) tester_v2.cpp tbb_queue/tbb_queue.cpp -o $(OUTDIR)/tbb_tester_v2.x $(LIBFLAGS)

TsigasQueue: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_TSIGAS_QUEUE=1 $(CXXFLAGS) $(LDFLAGS) tester_v2.cpp tsigas_queue/tsigas_queue.cpp -o $(OUTDIR)/tsigas_tester_v2.x $(LIBFLAGS)

LinuxBuffer: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_LINUX_BUFFER=1 $(CXXFLAGS) $(INCLUDES) tester_v2.cpp linux_buffer/lockfree_rb_q.cc -o $(OUTDIR)/linux_buffer.x $(LIBFLAGS)

LinuxBufferRecreate: tester_v2.cpp
	$(CC) $(OFLAGS) -DUSE_LINUX_BUFFER_RECREATE=1 $(CXXFLAGS) $(INCLUDES) tester_v2.cpp linux_buffer_recreate/linux_buffer_recreate.cpp -o $(OUTDIR)/linux_buffer_recreate.x $(LIBFLAGS)
	
clean:
	rm  $(OUTDIR)/*.x
