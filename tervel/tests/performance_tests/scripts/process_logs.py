__author__ = 'stevenfeldman'

import yaml
import os

def indent(str, depth = 1):
    temp = ""
    for x in range(0, depth, 1):
        temp += "\t"

    return str.replace("\n", temp)


# Add <metric_name, functions names> here to allow 'CustomMetrics' to be loaded
def add_operation(metric, obj):
    if metric.count == 0:
        for k in obj["Totals"]:
            metric.data[k] = 0
    metric.count += 1
    for k in obj["Totals"]:
        metric.data[k] +=  int(obj[k])

metric_functions = {"Operations" : add_operation}

class Metric:
    def __init__(self, n):
        self.name = n
        self.count = 0
        self.data = {}

    def add(self, obj):
        if self.name in metric_functions:
            metric_functions[self.name](self, obj)
        else:
            if self.count == 0:
                for k in obj:
                    self.data[k] = 0
            self.count += 1
            for k in obj:
                self.data[k] +=  float(obj[k])

    def __str__(self):
        str  = "name : " + self.name + "\n"
        str += "count : " + str(self.count) + "\n"
        str += "data : \n"
        for k in self.data:
            str += "  " + k + " : " + str(self.data[k]) + "\n"
        str += "data_avg \n"
        for k in self.data:
            str += "  " + k + " : " + (self.data[k] / self.count) + "\n"
        return str



class Execution:
    def __init__(self, obj):
        self.key = obj["CMD"]
        self.algorithm_name = obj["AlgorithmName"]
        self.algorithm_config = obj["AlgorithmConfig"]
        self.execution_time = obj["ExecutionTime"]
        self.main_delay = obj["MainDelay"]
        self.num_threads = obj["NumberThreads"]
        self.run_config = obj["RunConfig"]
        self.metrics = {}

        self.metrics["Operations"] =  Metric("Operations")


    def add_execution(self, obj):
        self.metrics["Operations"].add(obj["Operations"])
        if "CustomMetrics" in obj:
            for c in obj["CustomMetrics"]:
                if c not in self.metrics:
                    self.metrics[c] = Metric(c)
                self.metrics[c].add(obj["CustomMetrics"][c])

    def __str__(self):
        str = "key : " + self.key + "\n"
        str += "algorithm_name : " + self.algorithm_name + "\n"
        str += "algorithm_config : " + self.algorithm_config + "\n"
        str += "execution_time : " + self.execution_time + "\n"
        str += "main_delay : " + self.main_delay + "\n"
        str += "num_threads : " + self.num_threads + "\n"
        str += "run_config : " + self.run_config + "\n"
        for m in self.metrics:
            str += indent(m.__str__());

        return str

def load_file(execution_results, fname):
    with open(fname, "r") as fin:
        execution_result =  yaml.load(fin)

    cmd = execution_result["CMD"]
    if cmd not in execution_results:
        execution_results[cmd] = Execution(execution_result)
    execution_results[cmd].add_execution(cmd)

def load_logs_from_dir(execution_results, path):
    for (dirpath, dirnames, filenames) in os.walk(path):
        for f in filenames:
            load_file(execution_results, dirpath + f)
        break


execution_results = {}


for e in execution_results:
    print e
    print execution_results[e]