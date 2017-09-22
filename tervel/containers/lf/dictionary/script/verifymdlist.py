#!/usr/bin/python

import sys
import re


re_key = re.compile(r".*Key \[(\d+)\].*")
re_ignore = re.compile(r".*@@@@.*")


def main():
    import optparse

    parser = optparse.OptionParser(usage='\n\t%prog')

    (options, args) = parser.parse_args(sys.argv[1:])
    input_file = args[0]

    f = open(input_file)
    prev_key = 0
    key = 0
    l = 0
    key_count = 0
    suc = True
    for line in f:
        l += 1
        if re_ignore.match(line):
            continue
        match = re_key.match(line)
        if match:
            key_count += 1
            key = int(match.group(1))
            if key < prev_key:
                print "Unsorted keys {0} {1} on line {2}".format(prev_key, key, l)
                suc = False
                #break
            prev_key = key
            #print "Key {0}".format(key)
        else:
            print "[IGNORE LINE {0}] {1}".format(l, line.strip())
    if suc:
        print "All done, {0} keys are sorted".format(key_count)
    else:
        print "All done, {0} keys with some unsorted".format(key_count)

    f.close()


def find_lost():
    import optparse

    parser = optparse.OptionParser(usage='\n\t%prog')

    (options, args) = parser.parse_args(sys.argv[1:])
    input_file = args[0]

    key_count = dict()
    f = open(input_file)
    prev_key = 0
    key = 0
    l = 0
    suc = True
    for line in f:
        l += 1
        match = re_key.match(line)
        if match:
            key = int(match.group(1))
            if key in key_count:
                key_count[key] += 1
            else:
                key_count.update({key: 0})
            if key < prev_key:
                print "Unsorted keys {0} {1} on line {2}".format(prev_key, key, l)
                suc = False
                break
            prev_key = key
            #print "Key {0}".format(key)
        else:
            print "Unrecognized line {0}".format(l)
    if suc:
        for i in range(0, len(key_count)):
            if key_count[i] < 8:
                print "{0} keys counts {1}".format(i, key_count[i])
        print "Total {0} Key levels".format(len(key_count))

    f.close()


if __name__ == '__main__':
    main()

