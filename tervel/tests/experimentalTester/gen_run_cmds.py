import time


threads = [1,2,4,8,16,32,64]
exeTime = [20]
events = ["PAPI_TOT_INS", "PAPI_TOT_CYC", "PAPI_RES_STL", "PAPI_BR_INS"]

#print "dir=$(date +\"%T\")"
#print "mkdir $dir"

def ds_test(exe, exe_config, distributions):
#    print "mkdir $dir/" + exe + "/"
    for time in exeTime:
      for dist in distributions:
        for e in events:
          for thread in threads:
            cmd = ""
#           cmd = "bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh \""
            cmd += "./Executables/" + str(exe) + ".x -num_threads="+str(thread)
            cmd += " -execution_time="+str(time) + " " + exe_config
            cmd += " -main_sleep=10 "+str(thread) + " " + str(dist)
#            cmd += " \" " + str(time + 10)
#            cmd += " \"" + str(e) + "\""
            print cmd
#          print "mkdir $dir/" + exe + "/" + str(e)
#          print "mkdir $dir/" + exe + "/" + str(e) + "/data/"
#          print "mv /tmp/damian/ldmstest/* $dir/" + str(exe)+ "/"  + str(e) + "/data/"

def lfstack():
    distributions = ["50 50"] #, "40 60", "60 40"]
    ds_test("lf_stack", "-prefill=0", distributions)
def wfstack():
    distributions = ["50 50"] #, "40 60", "60 40"]
    ds_test("wf_stack", "-prefill=0", distributions)

def wfhashmapnodel():
    distributions = ["40 20 40"]
    ds_test("wf_hashmap_nodel", "-prefill=0 -capacity=32568 -expansion_factor=6", distributions)

# wfhashmapnodel()
wfstack()
lfstack()
