from __future__ import print_function
from datetime import datetime
import subprocess
import sys
import os
import glob
import time

os.chdir("Executables/")

while True:
    now = datetime.now()
    outFolder = os.path.join("run", now.strftime('%Y_%m_%d_%H%M'))
    if not os.path.exists(outFolder):
        os.makedirs(outFolder)
        break
    else:
        print("Error Time Stamp alreayd exists???")




threads=[2, 4, 8, 16, 32, 64, 96, 128, 160, 198, 256]
#enq, deq
#dists=[[100,0], [0,100], [50,50]] #[45,45,0,10,0,0], [40,40,0,20,0,0], [20,40,15,25,0,0], [40,40,5,15,0,0], [25,25,20,30,0,0], [40,40,0,20,0,0],[10,10,30,50,0,0],[15,15,5,65,0,0], [0,0,0,100,0,0] ]
dists=[[50, 50]]
prefills=[2097152]
capacitys=[4194304]

exeTimes=[5]
reps=3

fileList=glob.glob("*.x")
#fileList = ('linux_tester.x')

fcount=len(fileList)

time=reps*len(threads)*(fcount)*sum(exeTimes)*len(dists)*len(prefills)
print("Estimate Time: %d\n" %time, file=sys.stderr)
sys.stdout.flush()

header="Threads\t"
header+="\t".join(fileList)
print(header)

for prefill in prefills:
    for capacity in capacitys:
        for dist in dists:
            for exeTime in exeTimes:
                infoStr=("#Exe Time: {}, Capacity: {},  Prefill: {}, Enqueue: {}, Dequeue: {} ".format(exeTime, capacity, prefill, *dist) )
                print(infoStr)


                fname=("T{}_C{}_P{}_E{}_D{}".format(exeTime, capacity, prefill, *dist) )
                fout = open(outFolder+"/"+fname, 'w')
                fout.write(infoStr+"\n")
                fout.write(header+"\n")
                for t in threads:
                    fout.write(str(t)+"\t")
                    for currFile in fileList:
                        #fout.write ("=((0")
                        vSum=0.0
                        try:
                            total=0
                            for r in xrange(reps):
                                cmd = ["./"+currFile, exeTime, t, capacity]
                                cmd.extend(dist)
                                cmd.append(prefill)
                                p = subprocess.Popen([str(_) for _ in cmd], stdout=subprocess.PIPE)
                                out, err = p.communicate()
                                vSum+=float(out)
#                            fout.write("+"+out)
                        except Exception as e:
                            print("Exception was thrown" + str(cmd) + str(e))
                        finally:
     #                       fout.write(")/"+str(reps)+")\t")
                             vSum/=float(reps)
                             fout.write((str(vSum)+"\t"))

                    fout.write("\n")
                fout.flush()
                fout.close()

                command="gnuplot -e \"file='"+outFolder+"/"+fname+"'\" ../barplot.gp"
                #print(command)
                p = os.popen(command, "r")
                while 1:
                    line = p.readline()
                    if not line: break
                    print( line)

