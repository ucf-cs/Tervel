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

from __future__ import print_function
from datetime import datetime
import subprocess
import sys
import os
import glob
import time

os.chdir("Executables/")
outFolder = os.path.join("Results/");
if not os.path.exists(outFolder):
  os.makedirs(outFolder)

while True:
  now = datetime.now()
  fname = now.strftime('%Y_%m_%d_%H%M')

  if os.path.isfile(fname):
    print("Error Time Stamp alreayd exists???")
  else:
    break

fout = open(outFolder+"/"+fname, 'w')

header="ExeTime\tCapacity\tPrefill\tEnqueueRate\tExecutable\tThreads\tEnqueues\tDequeues"
fout.write(header+"\n")

threads=[2, 4, 8, 16, 32, 64, 96, 128, 160, 198, 256]
enqueue_rates=[50]
prefill_percents=[50]
capacitys=[4194304]

exeTimes=[1]
reps=3

fileList=glob.glob("*.x")

fcount=len(fileList)

time=reps*len(threads)*(fcount)*sum(exeTimes)*len(prefill_percents)*len(capacitys)*len(enqueue_rates)

print("Estimate Time: %d (seconds)\n" %time, file=sys.stderr)
sys.stdout.flush()

for prefill in prefill_percents:
  for capacity in capacitys:
    for enqueue_rate in enqueue_rates:
      for exeTime in exeTimes:
        for t in threads:
          for currFile in fileList:

            infoStr=("#Exe Time: {}, Capacity: {},  Prefill: {}, Enqueue Rate: {}, File: {}, Threads: {}."
                .format(exeTime, capacity, prefill, enqueue_rate, currFile, t) )
            print(infoStr)
            enqueues=0.0
            dequeues=0.0
            try:
              total=0
              for r in range(reps):
                cmd = ["./"+currFile]
                cmd.append("-execution_time="+str(exeTime))
                cmd.append("-num_threads="+str(t))
                cmd.append("-buffer_length="+str(capacity))
                cmd.append("-enqueue_rate="+str(enqueue_rate))
                cmd.append("-prefill="+str(prefill))

                out = subprocess.check_output([str(_) for _ in cmd], timeout=exeTime+15)

                out = str(out).split("'")[1]
                result = out.split("\\t")

                enqueues += float(result[0]);
                dequeues += float(result[1]);
            except Exception as e:
              print("Exception was thrown" + str(cmd) + str(e))
            finally:
              enqueues/=float(reps)
              dequeues/=float(reps)
              infoStr=("{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\n"
                .format(exeTime, capacity, prefill, enqueue_rate, currFile, t
                  ,enqueues, dequeues) )
              fout.write(infoStr)
              fout.flush()
fout.close()
