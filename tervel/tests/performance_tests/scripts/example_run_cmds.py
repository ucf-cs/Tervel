import time

exe_count = 0;
time_count = 0;
cmds  = "dir=logs/$(date +\"%s\")\n"
cmds += "mkdir $dir\n"

threads = [1,2,4,8,16,32,64]
exeTime = [5]
path = "../executables/"
def ds_test(exe, ds_config, distributions):
    global exe_count, time_count, cmds
    for time in exeTime:
        for dist in distributions:
            for thread in threads:
                exe_count += 1
                time_count += time
                cmds += "temp=$dir/$(date +\"%s\").log\n"
                exe_cmd  = "./" + str(exe)
                exe_cmd += " -main_sleep=0"
                exe_cmd += " -verbose"
                exe_cmd += "  -execution_time=" + str(time)
                exe_cmd += " " + ds_config
                exe_cmd += " -num_threads=" + str(thread)
                exe_cmd += " " + str(thread) + " " + str(dist)
                cmds += "echo \"CMD : " + exe_cmd + "\" > $temp\n"
                exe_cmd += " 2>&1 >> $temp"
                cmds += "echo \"" + exe_cmd + "\"\n"
                cmds += exe_cmd + "\n"

def stack(exe):
    prefills = [0, 1000]
    for p in prefills:
        distributions = ["50 50" , "40 60", "60 40"]
        ds_test(path+exe, "-prefill=" + str(p), distributions)


def wfhashmapnodel():
    distributions = ["40 20 40", "33 33 33", "20 40 40" "40 40 20"]
    ds_test(path+"wf_hashmap_nodel.x", "-prefill=0 -capacity=32568 -expansion_factor=6", distributions)

wfhashmapnodel()
stack("wf_stack.x")
stack("lf_stack.x")

print "echo Number of Tests: " + str(exe_count)
print "echo Estimated Time: " + str(time_count)
print cmds