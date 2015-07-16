import time

exe_count = 0;
time_count = 0;
cmds  = "dir=logs/$(date +\"%s\")\n"
cmds += "mkdir $dir\n"

repeat_test=1 #5
threads = [1,2, 64] #,4,8,16,32,64]
exeTime = [2]
path = "../executables/"

def gen_tests(algs_):
    for time in exeTime:
        for thread in threads:
            flags = " -main_sleep=0"
            flags += " -verbose"
            flags += "  -execution_time=" + str(time)
            flags += " -num_threads=" + str(thread)
            for algs in algs_:
                algs(flags, time, thread)


def add_run(exe, time, flags, thread, dist):
    global exe_count, time_count, cmds
    for i in range(0, repeat_test):
        exe_count += 1
        time_count += time
        cmds += "temp=$dir/$(date +\"%s\").log\n"

        exe_cmd = "./" + path + exe + " " + flags + " " + str(thread) + " " +dist

        cmds += "echo \"CMD : " + exe_cmd + "\" > $temp\n"

        exe_cmd += " 2>&1 >> $temp"
        cmds += "echo \"" + exe_cmd + "\"\n"
        cmds += exe_cmd + "\n"

def stack(flags_, time_, thread_):
    algs = ["wf_stack.x", "lf_stack.x"]
    prefills = [0, 1000]
    distributions = ["50 50" , "40 60", "60 40"]
    for a in algs:
        for dist in distributions:
            for p in prefills:
                flags = "-prefill=" + str(p)
                add_run(a, time_,flags + flags_, thread_, dist)


def ringbuffer(flags_, time_, thread_):
    algs = ["wfringbuffer.x", "lfmcasbuffer.x", "tbb.x", "lock.x", "tsigas.x", "linux.x"]
    prefills = [500]
    capacities = [1024]
    distributions = ["50 50"] # , "40 60", "60 40"]
    for a in algs:
        for dist in distributions:
            for c in capacities:
                for p in prefills:
                    flags = "-prefill=" + str(p) + " -capacity=" + str(c)
                    add_run(a, time_,flags + flags_, thread_, dist)

def hashmap(flags_, time_, thread_):
    algs = ["wf_hashmap.x", "wf_hashmap_nodel.x"]
    distributions = ["40 20 40 0", "33 33 33 0", "20 40 40 0", "40 40 20 0"]

    prefills = [0]
    capacities = [32568]
    expansion_factors = [6]
    for a in algs:
        for dist in distributions:
            for p in prefills:
                for e in  expansion_factors:
                    for c in capacities:
                        flags = "-prefill=" + str(p) +" -capacity="+str(c)+" -expansion_factor="+str(e)
                        add_run(a, time_, flags + flags_, thread_, dist)

algorithms = [ringbuffer] #hashmap, stack]
gen_tests(algorithms)
print "echo Number of Tests: " + str(exe_count)
print "echo Estimated Time: " + str(time_count)
print cmds
