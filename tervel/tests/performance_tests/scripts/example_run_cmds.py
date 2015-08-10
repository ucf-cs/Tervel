import time

main_sleep_time = 0
exe_count = 0;
time_count = 0;

if False:
  repeat_test=1
  threads = [1, 64] #,4,8,16,32,64]
  exeTime = [4]
else:
  repeat_test=5
  threads = [2,4,32,64]
  exeTime = [5]


path = "../executables"

pathFolder = path.split("/")
pathFolder = pathFolder[-1] if pathFolder[-1] else pathFolder[-2]
path += "" if path[-1] is "/" else "/"

test_commands = []
def gen_tests(algs_):
    for time in exeTime:
        flags  = " -main_sleep=%d" %main_sleep_time
        flags += " -verbose"
        flags += " -disable_thread_join"
        flags += " -execution_time=" + str(time)

        for algs in algs_:
            algs(flags, time)


def add_run(exe, time, flags, dist):
    global exe_count, time_count, cmds
    for i in range(0, repeat_test):
        exe_count += 1
        time_count += time + main_sleep_time
        exe_cmd = "likwid-pin -q -c 0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15,16,24,17,25,18,26,19,27,20,28,21,29,22,30,23,31,32,40,35,38,46,39,47,48,56,49,57,50,58,51,59,52,60,53,61,54,62,55,63 "
        exe_cmd += "./$exePath/%s %s %s" %(exe, flags, dist)
 #       exe_cmd += " -papi_events="
#        exe_cmd += "PAPI_TOT_INS,PAPI_TOT_CYC,INSTRUCTION_CACHE_INVALIDATED,MISALIGNED_ACCESSES"
        test_commands.append([exe_cmd, time + 20])

def stack(flags_, time_):
    algs = ["stack_tervel_wf.x", "stack_tervel_lf.x"]
    prefills = [0, 1000]
    distributions = []
    distributions.append(lambda t: None if t < 2 else "%d 100 0 %d 0 100" %((t*.5), (t*.5)))
    distributions.append(lambda t: None if t < 4 else "%d 100 0 %d 0 100" %((t*.25), (t*.75)))
    distributions.append(lambda t: None if t < 4 else "%d 100 0 %d 0 100" %((t*.75), (t*.25)))
    distributions.append(lambda t: None if t < 1 else "%d 50 50" %(t))
    distributions.append(lambda t: None if t < 1 else "%d 25 75" %(t))
    distributions.append(lambda t: None if t < 1 else "%d 75 25" %(t))

    for p in prefills:
        for dist in distributions:
            flags = " -prefill=" + str(p)

            for thread in threads:
                temp = flags_ + flags + " -num_threads=" + str(thread)
                if dist is None:
                    tdist = str(thread) + " 0 0"
                    temp += " -iter_dist "
                else:
                    tdist = dist(thread)
                    if not tdist:
                        continue

                for a in algs:
                    add_run(a, time_, temp, tdist)
            test_commands.append(None)


def ringbuffer(flags_, time_):
    # algs = ["buffer_tervel_wf.x", "buffer_linux_nb.x", "buffer_tbb_fg.x", "buffer_tsigas_nb.x", "buffer_lock_cg.x", "buffer_tervel_mcas_lf.x"] # "buffer_naive_cg.x",
    algs = ["buffer_tervel_wf.x", "buffer_tervel_mcas_lf.x"]
    prefills = [16384]#, 0, 32768]
    capacities = [32768]
    distributions = []
    distributions.append(None) # Alternate Test.

    distributions.append(lambda t: None if t < 2 else "%d 100 0 %d 0 100" %((t*.5), (t*.5)))
    distributions.append(lambda t: None if t < 4 else "%d 100 0 %d 0 100" %((t*.25), (t*.75)))
    distributions.append(lambda t: None if t < 4 else "%d 100 0 %d 0 100" %((t*.75), (t*.25)))
    distributions.append(lambda t: None if t < 1 else "%d 50 50" %(t))
    distributions.append(lambda t: None if t < 1 else "%d 25 75" %(t))
    distributions.append(lambda t: None if t < 1 else "%d 75 25" %(t))

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
                        if not tdist:
                            continue

                    for a in algs:
                        add_run(a, time_, temp, tdist)
                test_commands.append(None)

def hashmap(flags_, time_):
    algs = ["hashmap_nodel_tervel_wf.x", "hashmap_tervel_wf.x"]
    distributions = ["40 20 40 0", "33 33 33 0", "20 40 40 0", "40 40 20 0"]

    prefills = [0, 16384]
    capacities = [32768]
    expansion_factors = [6]


    for p in prefills:
        for e in  expansion_factors:
            for c in capacities:
                for dist in distributions:
                    for thread in threads:
                        flags = "-prefill=" + str(p) +" -capacity="+str(c)+" -expansion_factor="+str(e) + " -num_threads=" + str(thread) + " "
                        for a in algs:
                            add_run(a, time_, flags + flags_, str(thread) + " " + dist)

algorithms = [ringbuffer, stack]
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
print "dir=logs/$tStamp"
print "mkdir -p $dir"
print "cp -r %s $dir/" %(path)
print "cp ../Makefile $dir/"
print "cp example_run_cmds.py  $0 $dir/"
print "exePath=$dir/%s" %(pathFolder)

  #
i = 0
for c in test_commands:
    if c is None:
        print "tar -zvcf $tStamp.ss.tar.gz $dir/"
    else:
        print "temp=$dir/$(date +\"%s\").log"
        print "echo \"CMD : %s 2>&1 >> $dir/test_%d.log\" | tee $dir/test_%d.log" %(c[0], i,i)
        print "timeout %d %s 2>&1 >> $dir/test_%d.log" %(c[1], c[0], i)
        print "if [ $? -ne 0 ]; then"
        print "  echo \"\t killed\""
        print "fi"
        i += 1
print "tar -zvcf $tStamp.tar.gz $dir/"

