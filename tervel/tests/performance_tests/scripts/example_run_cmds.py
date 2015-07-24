import time

exe_count = 0;
time_count = 0;

if False:
  repeat_test=1
  threads = [2, 64] #,4,8,16,32,64]
  exeTime = [1]
else:
  repeat_test=20
  threads = [2,4,8,16,32,64]
  exeTime = [5]


path = "../executables"

pathFolder = path.split("/")
pathFolder = pathFolder[-1] if pathFolder[-1] else pathFolder[-2]
path += "" if path[-1] is "/" else "/"

test_commands = []
def gen_tests(algs_):
    for time in exeTime:
        flags  = " -main_sleep=0"
        flags += " -verbose"
        flags += " -disable_thread_join"
        flags += " -execution_time=" + str(time)

        for algs in algs_:
            algs(flags, time)


def add_run(exe, time, flags, dist):
    global exe_count, time_count, cmds
    for i in range(0, repeat_test):
        exe_count += 1
        time_count += time
        exe_cmd = "./$exePath/" + exe + " " + flags + " " + dist + " 2>&1 >> $temp"
        test_commands.append([exe_cmd, time + 20])

def stack(flags_, time_, thread_):
    algs = ["stack_tervel_wf.x", "stack_tervel_lf.x"]
    prefills = [0, 1000]
    distributions = ["50 50" , "40 60", "60 40"]

    for dist in distributions:
        for p in prefills:
            for thread in threads:
                flags = "-prefill=" + str(p) + " -num_threads=" + str(thread) + " "
                for a in algs:
                    dist = str(thread) + " " + dist
                    add_run(a, time_,flags + flags_, dist)


def ringbuffer(flags_, time_):
    algs = ["buffer_tervel_wf.x", "buffer_linux_nb.x", "buffer_tbb_fg.x", "buffer_tsigas_nb.x", "buffer_lock_cg.x", "buffer_tervel_mcas_lf.x"] # "buffer_naive_cg.x",
    prefills = [16384]#, 0, 32768]
    capacities = [32768]
    distributions = []
    distributions.append(None) # Alternate Test.

    distributions.append(lambda t: "%d 100 0 %d 0 100" %((t*.5), (t*.5)))
    distributions.append(lambda t: "%d 100 0 %d 0 100" %((t*.25), (t*.75)))
    distributions.append(lambda t: "%d 100 0 %d 0 100" %((t*.75), (t*.25)))
    distributions.append(lambda t: "%d 50 50" %(t))
    # distributions.append(lambda t: str(t) + " 20 80")
    # distributions.append(lambda t: str(t) + " 80 20")

    for c in capacities:
        for p in prefills:
            for dist in distributions:
                flags = " -prefill=" + str(p) + " -capacity=" + str(c)

                for thread in threads:
                    temp = flags_ + flags + " -num_threads=" + str(thread)
                    if dist is None:
                        tdist = str(thread) + " 0 0"
                        temp += " -iter_dist "
                    else:
                        tdist = dist(thread)

                    for a in algs:
                        add_run(a, time_, temp, tdist)
                test_commands.append(None)

def hashmap(flags_, time_):
    algs = ["hashmap_nodel_tervel_wf.x", "hashmap_tervel_wf.x"]
    distributions = ["40 20 40 0", "33 33 33 0", "20 40 40 0", "40 40 20 0"]

    prefills = [0]
    capacities = [32768]
    expansion_factors = [6]

    for dist in distributions:
        for p in prefills:
            for e in  expansion_factors:
                for c in capacities:
                    for thread in threads:
                        flags = "-prefill=" + str(p) +" -capacity="+str(c)+" -expansion_factor="+str(e) + " -num_threads=" + str(thread) + " "
                        for a in algs:
                            dist = str(thread) + " " + dist
                            add_run(a, time_, flags + flags_, dist)

algorithms = [ringbuffer] #hashmap, stack]
gen_tests(algorithms)

def humanize_time(secs):
    mins, secs = divmod(secs, 60)
    hours, mins = divmod(mins, 60)
    return '%02d:%02d:%02d' % (hours, mins, secs)

print "echo Number of Tests: " + str(exe_count)
time_count += exe_count*2
print "echo Min Estimated Time: %s" %(humanize_time(time_count))
time_count += exe_count*20
print "echo Max Estimated Time: %s" %(humanize_time(time_count))

print "tStamp=$(date +\"%s\")"
print "dir=logs/buffer/$tStamp"
print "mkdir -p $dir"
print "cp -r %s $dir/" %(path)
print "exePath=$dir/%s" %(pathFolder)
for c in test_commands:
    if c is None:
        print "tar -zvcf $tStamp.ss.tar.gz $dir/"
    else:
        print "temp=$dir/$(date +\"%s\").log"
        print "echo \"CMD : " + c[0] + "\" | tee $temp"
        print "timeout " + str(c[1]) + " " + c[0]
        print "if [ $? -ne 0 ]; then"
        print "  echo \"\t killed\""
        print "fi"
print "tar -zvcf $tStamp.tar.gz $dir/"

