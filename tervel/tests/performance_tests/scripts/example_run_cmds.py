import time


threads = [1,2,4,8,16,32,64]
exeTime = [20]

def ds_test(exe, exe_config, distributions):
    for time in exeTime:
      for dist in distributions:
          for thread in threads:
            cmd = ""
            cmd += "./../executables/" + str(exe) + ".x -num_threads="+str(thread)
            cmd += " -main_sleep=0  -execution_time="+str(time) + " " + exe_config
            cmd += " " + str(thread) + " " + str(dist)
            cmd += " 2>&1 > $dir/$(date +\"%s\").log"
            print cmd

def lfstack():
    distributions = ["50 50"] #, "40 60", "60 40"]
    ds_test("lf_stack", "-prefill=0", distributions)
def wfstack():
    distributions = ["50 50"] #, "40 60", "60 40"]
    ds_test("wf_stack", "-prefill=0", distributions)

def wfhashmapnodel():
    distributions = ["40 20 40"]
    ds_test("wf_hashmap_nodel", "-prefill=0 -capacity=32568 -expansion_factor=6", distributions)



print "dir=logs/$(date +\"%s\")"
print "mkdir $dir"


wfhashmapnodel()
wfstack()
lfstack()
