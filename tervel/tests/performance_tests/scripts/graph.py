__author__ = 'stevenfeldman'
import sys
import yaml
import os
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np

# Add <metric_name, functions names> here to allow 'CustomMetrics' to be loaded
def add_operation(data, name, obj):
    sum = 0
    for k in obj["totals"]:
        temp = int(obj["totals"][k])
        sum += temp

    temp = {"total":sum, "count":1 }

    Execution.add_key_val_pair(data, temp, name)


metric_functions = {"operations": add_operation}


class Execution:
    @staticmethod
    def add_key_val_pair(data, set_src, prefix_=""):
        for s in set_src:
            res = prefix_ + ":" + s
            val = set_src[s]
            if type(val) is dict or type(val) is set:
                Execution.add_key_val_pair(data, val, res)
            elif res not in data:
                data[res] = val
            else:
                data[res] += val

    @staticmethod
    def process(obj):
        data = {"key":""}

        test_config = obj["test_config"]
        alg_config = obj["alg_config"]
        alg_config["run_config"] = " ".join(alg_config["run_config"].split(" ")[1:])

        Execution.add_key_val_pair(data, test_config, "test_config")
        Execution.add_key_val_pair(data, alg_config, "alg_config")

        return data

    @staticmethod
    def add_metric(data, name, obj):
        if name in metric_functions:
            metric_functions[name](data, "metrics:"  + name, obj)
        else:
            obj["count"] = 1
            Execution.add_key_val_pair(data, obj, "metrics:" + name)

    @staticmethod
    def add_execution(data, obj):
        if "metrics" in obj:
            for c in obj["metrics"]:
                Execution.add_metric(data, c, obj["metrics"][c])


def load_file(results, fname):
    with open(fname, "r") as fin:
        execution_result = yaml.load(fin)

    cmd = execution_result["CMD"]
    if cmd not in results:
        results[cmd] = Execution.process(execution_result)

    data = results[cmd]
    Execution.add_execution(data, execution_result)


def load_logs_from_dir(results, path):
    for (dirpath, dirnames, filenames) in os.walk(path):
        num_files = len(filenames)
        c = 0
        for f in filenames:
            c += 1
            f = dirpath + "/" + f
            print "(" + str(c) + "/" + str(num_files) + "):  " + f
            load_file(results, f)
        break


if __name__ == "__main__":
    execution_results = {}
    load_logs_from_dir(execution_results, "logs/1435363336/")

    df = pd.DataFrame(execution_results.values())

    for k in df.keys():
        if "metrics:" in k and ":count" not in k:
            # metrics:<metric name>:count
            count_str = "metrics:" + k.split(":")[1] + ":count"
            df[k] /= df[count_str]

    config = "test_config:num_threads"
    # metric = "metrics:PAPIResults:PAPI_L1_TCA"
    metric = "metrics:operations:total"


    df_filtered = df #[(df["alg_config:algorithm_name"].str.contains("Stack"))]
    df_filtered = df[(df["alg_config:algorithm_name"].str.contains("Map") or df["alg_config:algorithm_name"].str.contains("map"))]
    #df_filtered = df_filtered[(df["alg_config:run_config"].str.contains("50 50"))]
    # df_filtered = df_filtered[(df["alg_config:ds_config:Prefill"] == 0)]

    for k in df_filtered.keys():
        if k != config and "metrics" not in k and k != "key":

            def func(row):
                temp = str(row[k])
                if temp != "nan":
                    i = k.rfind(":") + 1
                    return k[i:] + ":" + temp + "_"
                return ""
            df_filtered["key"] += df_filtered.apply(lambda row:  func(row), axis=1)

    plt.figure(1)

    sns.set(style="ticks")
    ax = sns.barplot(config,  metric, "key", data=df_filtered)

    handles, labels = ax.get_legend_handles_labels()
    # lgd = ax.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5,-2.0))

    #
    # plt.figlegend(handles, labels, loc = 'lower center', ncol=1, labelspacing=0. )
    # plt.show()
    # sns.plt.show()

    sns.plt

    # Shrink current axis's height by 10% on the bottom
    box = ax.get_position()
    shrink = .5
    ax.set_position([box.x0, box.y0 + box.height * (1-shrink),
                     box.width, box.height * shrink])

    # Put a legend below current axis
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, -.25),
              fancybox=True, shadow=True, ncol=1)

    plt.show()