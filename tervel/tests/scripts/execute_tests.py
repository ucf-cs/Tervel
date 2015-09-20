from __future__ import print_function
import sys
import time
import os
import os.path
import socket
import itertools
import subprocess
timeout = 20

def humanize_time(secs):
    mins, secs = divmod(secs, 60)
    hours, mins = divmod(mins, 60)
    return '%02d:%02d:%02d' % (hours, mins, secs)

def alt_spawn(threads, rate1, rate2):
    #TODO: make this dynamic
    s = ""
    for t in range(0, threads, 2):
        s += "1 0 %d 1 %d 0 " %(rate1, rate1)
    return s

class Tester:
    def __init__(self, config):
        self.num_tests = 0
        self.test_fin_count = 0
        self.min_time = 0
        self.max_time = 0


        if config is None:
            raise ValueError('config variable must not be None')
        if 'description' not in config:
            config['description'] = raw_input("Enter test description: ")
        if 'main_sleep_time' not in config:
            config['main_sleep_time'] = 0
        if 'exe_repetition' not in config:
            config['exe_repetition'] = 1
        if 'exe_time' not in config:
            config['exe_time'] = [1]
        if 'system_name' not in config:
            config['system_name'] = socket.gethostname()
        if 'papi_flag' not in config:
            config['papi_flag'] = None
        if 'misc_flags' not in config:
            config['misc_flags'] = None
        if 'thread_levels' not in config:
            config['thread_levels'] = [1]
        if 'exe_path' not in config:
            config['exe_path'] = '../executables'
        if 'verbose' not in config:
            config['verbose'] =  False
        if 'disable_thread_join' not in config:
            config['disable_thread_join'] = False
        if 'exe_prefix' not in config:
            config['exe_prefix'] = ""


        if 'log_directory' not in config:
            config['log_directory'] = ""

        while True:
            temp = os.path.join(config['log_directory'], "%d" %(time.time()))
            if not os.path.exists(temp):
                os.makedirs(temp)
                config['log_directory'] = temp
                os.makedirs(os.path.join(config['log_directory'], "output"))
                break

        if 'log_git' in config and config['log_git']:
            f = os.path.join(config['log_directory'], "git_status.log")
            with open(f, "w") as fout:
                subprocess.check_call(["git", "status"], stdout=fout, stderr=fout)
            f = os.path.join(config['log_directory'], "git_diff.log")
            with open(f, "w") as fout:
                subprocess.check_call(["git", "diff"], stdout=fout, stderr=fout)
            f = os.path.join(config['log_directory'], "git_log.log")
            with open(f, "w") as fout:
                subprocess.check_call(["git", "log", "-1"], stdout=fout, stderr=fout)

        self.config = config

    def log_preamble(self, cmd, test_num):
        s  = "CMD : %s\n" % cmd
        s += "TestNum : %d\n" % test_num
        s += "SYSTEM : %s\n" % self.config['system_name']
        s += "DESCR : %s\n" % self.config['description']
        return s

    def const_flags(self):
        if not hasattr(self, 'cflags'):
            self.cflags = "-main_sleep=%d" % self.config['main_sleep_time']
            if self.config['papi_flag'] is not None:
                self.cflags += " %s" % self.config['papi_flag']
            if self.config['misc_flags'] is not None:
                self.cflags += " %s" % self.config['misc_flags']
            if self.config['verbose'] is True:
                self.cflags += " -verbose"
            if self.config['disable_thread_join'] is True:
                self.cflags += " -disable_thread_join"
        return self.cflags

    def exe_prefix(self):
        return self.config['exe_prefix']

    def getTestFlags(self, t):
        tflags = t['flags']
        if not tflags or len(tflags) == 0:
            return [""]

        res = []
        for k in tflags.keys():
            temp = []
            if len(tflags[k]) == 0:
                continue
            for v in tflags[k]:
                temp.append("-%s=%s" %(k, str(v)))
            res.append(temp)

        res2 = []
        for r in list(itertools.product(*res)):
            res2.append(" ".join(r))
        return res2

    def run_cmd(self, cmd, exe_time):
        logfile = os.path.join(self.config['log_directory'], "output", "test_%d.log" %(self.test_fin_count))


        s = "[%d/%d](%s - %s) %s" %(self.test_fin_count, self.num_tests, humanize_time(self.min_time), humanize_time(self.max_time), cmd)
        print("\r" + s + " " + logfile)



        with open(logfile, 'w') as fout:
            fout.write(self.log_preamble(cmd, self.test_fin_count))
            cmd = "timeout %d %s 2>&1 >> %s" %(exe_time + timeout, cmd, logfile)
            subprocess.check_call(cmd.split(" "), stdout=fout, stderr=fout)

        # Update Progress Counts
        self.test_fin_count += 1
        self.min_time -= exe_time
        self.max_time -= exe_time + timeout

    def run_test(self, executable, flag, dist, thread, exe_time, time_count):

        if dist is None:
            # TODO implement this code
            # dist = ?
            return
        else:
            dist = dist(thread)

        cmd = self.exe_prefix()
        cmd += " %s" %(executable)
        cmd += " %s" %(flag)
        cmd += " -num_threads=%d %s" %(thread, dist)

        reps = self.config['exe_repetition']
        for rep in range(0, reps, 1):
            if time_count:
                self.num_tests += 1
                self.min_time += exe_time
                self.max_time += exe_time + timeout
            else:
                self.run_cmd(cmd, exe_time)

    def run_tests(self):
        self.run_tests_(time_count=True)
        self.run_tests_(time_count=False)

    def run_tests_(self, time_count):
        for exe_time in self.config['exe_time']:
            for t in self.config['tests']:
                for flag in self.getTestFlags(t):
                    flag += " -execution_time=%d %s" %(exe_time, self.const_flags())
                    for dist in t['dist']:
                        for executable in t['executables']:
                            executable = "./%s " %(os.path.join(t['path'], executable))
                            for thread in self.config['thread_levels']:
                                self.run_test(executable, flag, dist, thread, exe_time, time_count)
                            print ("generating archive...")
                            subprocess.check_call(["tar", "-zcf", os.path.join(self.config['log_directory'], "log.tar.gz"), os.path.join(self.config['log_directory'], "output")])


configFile =  raw_input("Enter config file name: ")
if configFile is "":
    configFile = "example.config"

with open(configFile, 'r') as fin:
    test_config = eval(fin.read())
test = Tester(test_config)
print ("Log Directory: %s\n" %(test.config['log_directory']))
test.run_tests()


