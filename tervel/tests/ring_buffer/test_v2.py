from __future__ import print_function
from datetime import datetime
import subprocess
import sys
import os
import glob
import time

os.chdir("Executables_v2/")


now = datetime.now()
outFolder = os.path.join("run", now.strftime('%Y_%m_%d_%H%M'))

count=0
while True:
    
    if not os.path.exists(outFolder):
        os.makedirs(outFolder)
        break
    else:
        count+=1
        outFolder = os.path.join("run", now.strftime('%Y_%m_%d_%H%M')+"-"+str(count))


# [totalThreads [nthreads, enq, deq] ]

executionPatterns= [
    [1,  [1,50,50] ],
    [2,  [2,50,50] ],
    [4,  [4,50,50] ],
    [8,  [8,50,50] ],
    [16,  [16,50,50] ],
    [32,  [32,50,50] ],
    [64,  [64,50,50] ],
    [128,  [128,50,50] ],
    [256,  [256,50,50] ],
]


prefills=[2097152]
sizes=[4194304]
exeTimes=[1]
reps=3

fileList=glob.glob("*.x")

fcount=len(fileList)

time=reps*len(executionPatterns)*(fcount)*sum(exeTimes)*len(sizes)*len(prefills)
print("Estimate Time: %d.\n" %time, file=sys.stderr)
sys.stdout.flush()

header="Threads,"
header+="\t".join(fileList)

fout = open(outFolder+"/result.csv", 'w')
fout.write("Size, Execution Time, Prefill, Total Threads, Distribution, Algorithm, Enqueues, Dequeues, Successful Enqueues, Successful Dequeues\n")

for size in sizes:
    for prefill in prefills:
        for exeTime in exeTimes:
            for exe in executionPatterns:
#Order: Time, size, prefll, Total Threads, [NThreads, EnqRate, DeqRate]*

                command=[exeTime, size, prefill, exe[0]]
                for e in exe[1:]:
                    command.extend(e)        
            
                for fname in fileList:
                    shortFname=fname
                    if shortFname.find("/") != -1:
                        shortFname=shortFname[shortFname.find("/")]
                    if shortFname.rfind(".x") != -1:
                        shortFname=shortFname[0:shortFname.find(".x")]

                    total=[0.0] *4

                    cmd = ["./"+fname]
                    cmd.extend(command)
                    cmd=[str(_) for _ in cmd]
                    
                    print(cmd)
                    try:
                        for r in xrange(reps):
                            
                            p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
                            out, err = p.communicate()
                            
                            results=out.split(",")
                            
                            for i in xrange(len(results)):
                                total[i]+=float(results[i])
#                            fout.write("+"+out)
                    except:
                        print(out)
                        print(err)
                        print (sys.exc_info()[0])
                    finally:
                        for i in xrange(len(total)):
                            total[i]/=float(reps)
                        strres="{},{},{},\"{}\",\"{}\",{},{},{},{},{}\n" .format(size,exeTime, prefill, exe[0], exe[1], shortFname, *total)
                        fout.write(strres)

fout.write("\n")
fout.flush()
fout.close()
