__author__ = 'stevenfeldman'
import yaml
import os
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

class Execution:
    def __init__(self, t, o, st,et):
        self.threads = t
        self.ops = o
        self.start_time = st
        self.end_time = et
        self.events = {}

    def key(self):
        return str(self.threads)

    def sanity(self):
        print "Threads: %s\n", self.threads
        print "Ops: %s\n", self.ops
        for k in self.events.keys():
            print k, self.events[k]

def process_dir(data, path):
    with open(path+"/logs/app.log", "r") as f:
        exelog = yaml.load(f)
        threads = exelog["NumberThreads"]
        st = exelog["Time"]["start"]
        et = exelog["Time"]["end"]
        st = float(st)
        et = float(et)
        temp = exelog["Totals"]
        ops = 0
        for k in temp.keys():
            ops += temp[k]

        exe = Execution(threads, ops, st, et)

    with open(path+"/store/node/papi", "r") as f:
        l = f.readline()
        headers = l.split(",")
        for x in range(0, len(headers)):
            temp = headers[x]
            if "Time" not in temp:
                temp = temp.split("_",1)[1]
                temp = temp.replace("/","_").strip()
                headers[x] = temp

            if temp not in exe.events:
                exe.events[temp] = []

        while True:
            l = f.readline()
            if not l:
              break
            values = l.split(",")
            time = float(values[0].strip())
            if time < exe.start_time or time > exe.end_time:
                continue

            for k in exe.events.keys():
                exe.events[k].append(0)


            temp = float(values[0].strip()) - exe.start_time
            lastPos = len(exe.events[headers[0]]) - 1
            exe.events[headers[0]][lastPos] += temp
            for x in range(1, len(values)):
                temp = float(values[x].strip())
                lastPos = len(exe.events[headers[x]]) - 1
                exe.events[headers[x]][lastPos] += temp

        data[exe.key()] = exe

def process_folder(path):
    data = {}
    dirs = os.walk(path).next()[1]
    for d in dirs:
        process_dir(data, path+d)
    return data

tests = ["1", "2", "4", "8", "16", "32", "64"]
nTests = len(tests)

def process_data(data, path):
    fout = open(path+"log.txt", 'w')
    for k in data.keys():
        metrics = data[k].events.keys()
        break

    fout.write("Thread-Level\tValue\t This/Thread-Level 1\n")
    for k in metrics:
        if ".CompId" in k:
            continue

        plt.ylabel(k)
        plt.xlabel('Seconds')

        ls = [":", "-", "--", "-."]

        fout.write("%s:\n" %k)
        for x in range(0, nTests):
            temp1 = data[tests[x]].events[k]
            timeArray = data[tests[x]].events["#Time"]

            plt.plot(timeArray, temp1, ls[x%len(ls)], label="Threads: %s" % tests[x])

            a = temp1[len(temp1) -1]
            b = a / data["1"].events[k][len(data["1"].events[k]) - 1]
            fout.write("\t%s\t%d\t%f\n" %(tests[x], a, b))

        lgd = plt.legend(ncol=3, loc='upper center', bbox_to_anchor=(0.5,-0.105))

        plt.savefig(path+'%s.pdf' %(k), bbox_extra_artists=(lgd,), bbox_inches='tight')  # saves the current figure into a pdf page

        plt.clf()

    fout.write("WorkDone:\n")
    fout.write("#Threads & Operations Completed & Performance Change \\ \hline \n")
    one = float(data["1"].ops)
    for t in tests:
        exe = data[t]
        ops = exe.ops
        factor = float(ops) / (one)
        if factor < 0:
            factor = -1.0/factor
        fout.write("  %s & %d & %f \\ \hline\n" %(t, ops, factor))

    fout.close()

folder = "Stack/"
dirs = os.walk(folder).next()[1]
for d in dirs:
    path=folder+d+"/"
    data = process_folder(path+'data/')
    if not os.path.exists(path+"out/"):
        os.makedirs(path+"out/")
    process_data(data, path+"out/")