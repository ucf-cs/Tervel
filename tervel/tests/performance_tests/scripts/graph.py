__author__ = 'stevenfeldman'
import sys
import pickle
import yaml
import os
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np

# Add <metric_name, functions names> here to allow 'CustomMetrics' to be loaded
def add_operation(data, name, obj):
    sum_t = 0
    sum_f = 0
    sum_p = 0
    for k in obj["totals"]:
        temp = int(obj["totals"][k])
        sum_t += temp
        if "Fail" in k:
            sum_f += temp
        if "Pass" in k:
            sum_p += temp

    temp = {"total:all": sum_t,
            "total:pass": sum_p,
            "total:fail": sum_f
            }


    Execution.add_key_val_pair(data, temp, name)
    Execution.add_key_val_pair(data, obj["totals"], name)


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
    def add_metric(data, name, obj):
        if name in metric_functions:
            metric_functions[name](data, "metrics:" + name, obj)
        else:
            Execution.add_key_val_pair(data, obj, "metrics:" + name)

    @staticmethod
    def add_execution(data, obj):
        if "metrics" in obj:
            for c in obj["metrics"]:
                Execution.add_metric(data, c, obj["metrics"][c])


def load_file(fname):
    with open(fname, "r") as fin:
        obj = yaml.load(fin)

    test_config = obj["test_config"]
    alg_config = obj["alg_config"]
    temp = alg_config["run_config"].split(" ")
    alg_config["run_config"] = ""
    threads = float(obj["test_config"]["num_threads"])
    # if "num_op_types" in test_config:
    #     op_types = int(test_config["num_op_types"])
    # else:
    op_types = 2
    i = 0
    while i < len(temp):
        alg_config["run_config"] += "[%.2f%%:" %( float(temp[i])/float(threads) )
        i += 1
        for j in range(op_types):
            alg_config["run_config"] += " %d" %( int(temp[i]) )
            i += 1
        alg_config["run_config"] += "]"

    data = {}

    data["CMD"] = obj["CMD"]
    data["key"] = ""
    Execution.add_key_val_pair(data, test_config, "test_config")
    Execution.add_key_val_pair(data, alg_config, "alg_config")

    if "metrics" in obj:
        for c in obj["metrics"]:
            Execution.add_metric(data, c, obj["metrics"][c])
        return data
    else:
        return None

def load_logs_from_dir(path):
    results = []
    for (dirpath, dirnames, filenames) in os.walk(path):
        num_files = len(filenames)
        c = 0
        for f in filenames:
            c += 1
            f = dirpath + "/" + f
            print "(" + str(c) + "/" + str(num_files) + "):  " + f
            if ".log" in f:
                res = load_file(f)
                if (res != None):
                    results.append(res)
        break
    return results


if __name__ == "__main__":
    reprocess = False
    folder = "logs/buffer/1437427952/"
    pFile = folder + "df_1.p"
    if reprocess or not os.path.exists(pFile):
        execution_results = load_logs_from_dir( folder)
        print "Elements:" + str(len(execution_results))
        df = pd.DataFrame(execution_results)

        pickle.dump(df, open(pFile, "wb"))
    else:
        df = pickle.load(open(pFile, "rb"))


    config = "test_config:num_threads"
    # metric = "metrics:PAPIResults:PAPI_L1_TCA"
    # metric = "metrics:operations:total:all"
    # metric = "metrics:operations:total:pass"
    metric = "metrics:operations:total:fail"

    # for k in df.keys():
    for k in ["alg_config:algorithm_name", "alg_config:run_config", "alg_config:ds_config:prefill"]:
        if k != config and "metrics" not in k and k != "key" and k != "CMD" :

            def func(row):
                temp = str(row[k])
                if temp != "nan":
                    i = k.rfind(":") + 1
                    return k[i:] + ":" + temp + "_"
                return ""
            df["key"] += df.apply(lambda row:  func(row), axis=1)
    #

    print df.keys()

    # df = df[(df["alg_config:algorithm_name"].str.contains("MCAS"))]
    # df = df[(df["alg_config:run_config"].str.contains("100 0"))]
    # df = df[(df["test_config:num_threads"] == 2)]
    # df = df[(df["alg_config:ds_config:prefill"]==0)]
    df = df[(df["alg_config:ds_config:prefill"] == 16384)]

    # for i, v in df.iterrows():
    #     print str(i) + " " + str(v["key"])
    #     for k in df.keys():
    #         if "metrics" in k:
    #             print "\t" + k + " : " + str(v[k])

    plt.figure(1)

    sns.set(style="ticks")
    ax = sns.barplot(config,  metric, "key", data=df)

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