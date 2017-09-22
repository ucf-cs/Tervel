#!/usr/bin/python

import sys
import os
import re


re_time = re.compile(r"CPU Time: (.*?)s Wall Time: (.*?)s")


def main():
    import optparse

    parser = optparse.OptionParser(usage='\n\t%prog')

    (options, args) = parser.parse_args(sys.argv[1:])
    input_program = args[0]

    pq_dict = {0: "MDLIST",
               1: "CTRBST",
               2: "FRLIST",
               3: "RTLIST",
               4: "BRNBST",
               5: "ELNBST",
               6: "HLLIST"}

    iteration = 1000000
    key_range = 1000000000
    insertion = 9
    deletion = 1
    average = 3
    cpu_time_all = []
    wall_time_all = []
    for pq_type in [0, 1, 2, 3, 4, 5, 6]:
    #for pq_type in [5, 6]:
        cpu_time_perpq = []
        wall_time_perpq = []
        for thread in [1, 2, 4, 8, 16, 32, 64, 128]:
        #for thread in [1]:
            cpu_time_perthread = []
            wall_time_perthread = []
            cpu_time = 0.0
            wall_time = 0.0
            for i in xrange(0, average):
                pipe = os.popen(input_program + " {0} {1} {2} {3} {4} {5}".format(pq_type, thread, iteration, key_range, insertion, deletion))
                for line in pipe:
                    match = re_time.match(line)
                    if match:
                        cpu_time = cpu_time + float(match.group(1)) / average
                        wall_time = wall_time + float(match.group(2)) / average
                        print pq_dict[pq_type] + " Thread {0} Iteration {1}".format(thread, i + 1) + " Wall Time: {0}".format(match.group(2))
            cpu_time_perthread.append(str(cpu_time))
            wall_time_perthread.append(str(wall_time))
            cpu_time_perpq.append(cpu_time_perthread)
            wall_time_perpq.append(wall_time_perthread)
            #cpu_time_all.append(cpu_time_perthread)
            #wall_time_all.append(wall_time_perthread)
        #f = open('cputime_' + pq_dict[pq_type] + '_insertion_' + str(portion), 'wb')
        #for t in cpu_time_perpq:
            #f.write(', '.join(t))
            #f.write(',\n')
        #f.close()
        f = open('walltime_' + pq_dict[pq_type] + '_key_' + str(key_range) + '_ins_' + str(insertion) + '_del_'+ str(deletion), 'wb')
        thread = 1
        for t in wall_time_perpq:
            f.write(str(thread) + ', ')
            f.write(', '.join(t))
            f.write(',\n')
            thread = thread * 2
        f.close()
    #f = open('cputime_all_thread_scale', 'wb')
    #for t in cpu_time_all:
        #f.write(', '.join(t))
        #f.write(',\n')
    #f.close()
    #f = open('cputime_all_contention_scale', 'wb')
    #for c in zip(*cpu_time_all):
        #f.write(', '.join(c))
        #f.write(',\n')
    #f.close()
    #f = open('walltime_all_thread_scale', 'wb')
    #for t in wall_time_all:
        #f.write(', '.join(t))
        #f.write(',\n')
    #f.close()
    #f = open('walltime_all_contention_scale', 'wb')
    #for c in zip(*wall_time_all):
        #f.write(', '.join(c))
        #f.write(',\n')
    #f.close()


if __name__ == '__main__':
    main()
