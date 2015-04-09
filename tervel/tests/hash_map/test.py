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
from datetime import timedelta
import subprocess
import sys
import os
import glob
import time
py_ver = sys.version_info[0]
if (int(py_ver) < 3):
  print("Error: Need python >=3, Current version " + (sys.version))
  exit()

os.chdir("Executables/")
outFolder = os.path.join("Results/");
if not os.path.exists(outFolder):
  os.makedirs(outFolder)

while True:
  now = datetime.now()
  fname = now.strftime('%Y_%m_%d_%H_%M_%S')

  if not os.path.exists(outFolder+fname+"/"):
    outFolder  = outFolder+fname+"/"
    os.makedirs(outFolder)
    break;
  else:
    print("Error Time Stamp already exists???")





if True:
  threads = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 16, 32, 64]
  #threads = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
  #threads = [1, 2, 3, 4, 5, 6]
  op_rates = [ [10,18,70,2], [10,70,18,2], [10,88,0,2], [25,25,25,25], [34,33,0,33], [88,8,2,2], [88,10,0,2] ]
  prefill_percents = [50]
  capacitys = [1024]
  exeTimes = [5]
  reps = 3

else:
  threads = [2]
  op_rates = [ [10, 20, 30, 40]] #find, insert, update, remove
  prefill_percents = [100]
  capacitys = [1000]
  exeTimes = [2]
  reps = 1


fileList = glob.glob("*.x")
fileList.extend(glob.glob("*.class"))
fcount = len(fileList)

time = reps*len(threads)*(fcount)*sum(exeTimes)*len(prefill_percents)*len(capacitys)*len(op_rates)
time_str = datetime.now()
print ("Current Time %s" % str(time_str))
print("Estimate Time: %d (seconds)" %time, file=sys.stderr)
time_str = time_str + timedelta(0,time)
print ("Estimate completed time: %s" % str(time_str))
sys.stdout.flush()

for prefill in prefill_percents:
  for capacity in capacitys:
    for op_rate in op_rates:
      for exeTime in exeTimes:
        for t in threads:
          for currFile in fileList:
            for r in range(reps):
              try:
                fname = "F_" + currFile.split(".")[0]
                fname += "_Time_" + str(exeTime)
                fname += "_Cap_" + str(capacity)
                fname += "_Threads_" + str(t)
                fname += "_Ops_" + str(op_rate)
                fname += "-" + str(r)+ ".log"
                fout = open(outFolder+fname, 'w')

                if (".class" in currFile ):
                    cmd = ["java", "-classpath", ".:../java_hashmap/high-scale-lib/lib/high-scale-lib.jar:../java_hashmap/:"]
                    cmd.append(currFile.split(".")[0])
                    cmd.append(str(capacity))
                    cmd.append(str(t))
                    cmd.append(str(exeTime))
                    cmd.append(str(prefill))
                    cmd.append(str(op_rate[0]))
                    cmd.append(str(op_rate[1]))
                    cmd.append(str(op_rate[2]))
                    cmd.append(str(op_rate[3]))
                else:
                    cmd = ["./"+currFile]
                    cmd.append("--execution_time="+str(exeTime))
                    cmd.append("-num_threads="+str(t))
                    cmd.append("--capacity="+str(capacity))
                    cmd.append("--prefill="+str(prefill))
                    cmd.append("--find_rate="+str(op_rate[0]))
                    cmd.append("--insert_rate="+str(op_rate[1]))
                    cmd.append("--update_rate="+str(op_rate[2]))
                    cmd.append("--remove_rate="+str(op_rate[3]))

                out = subprocess.check_output([str(_) for _ in cmd], timeout=exeTime+15)
                fout.write(out.decode("utf-8"))

              except Exception as e:
                fout.write(" ".join(cmd) + "\n")
                fout.write("Exception was thrown" + str(cmd) + str(e)+"\n\n")
              finally:

                fout.flush()
                fout.close()
