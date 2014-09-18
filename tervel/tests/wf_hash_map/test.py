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
    print("Error Time Stamp already exists???")
  else:
    break

fout = open(outFolder+"/"+fname, 'w')

if False:
  threads=[1, 2, 4, 8, 16, 32, 64, 96, 128, 160, 198, 256]
  op_rates=[50] #find, insert, update, remove
  prefill_percents=[50]
  capacitys=[4194304]
  exeTimes=[1]
  reps=5
else:
  threads=[2]
  op_rates=[ [10, 20, 30, 40]] #find, insert, update, remove
  prefill_percents=[100]
  capacitys=[1000]
  exeTimes=[2]
  reps=1


fileList=glob.glob("*.x")

fcount=len(fileList)

time=reps*len(threads)*(fcount)*sum(exeTimes)*len(prefill_percents)*len(capacitys)*len(op_rates)

print("Estimate Time: %d (seconds)\n" %time, file=sys.stderr)
sys.stdout.flush()

for prefill in prefill_percents:
  for capacity in capacitys:
    for op_rate in op_rates:
      for exeTime in exeTimes:
        for t in threads:
          for currFile in fileList:
            try:
              for r in range(reps):
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



            except Exception as e:
              fout.write(" ".join(cmd) + "\n")
              fout.write("Exception was thrown" + str(cmd) + str(e)+"\n\n")
            finally:
              fout.write( out.decode("utf-8"))
              fout.flush()
fout.close()
