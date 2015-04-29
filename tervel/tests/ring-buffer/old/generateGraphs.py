#/*
#The MIT License (MIT)
#
#Copyright (c) 2015 University of Central Florida's Computer Software Engineering
#Scalable & Secure Systems (CSE - S3) Lab
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.
#
#*/

import os
import sys
import subprocess as sp
import re
for root, dirs, files in os.walk("Executables/run/"):
    for fname in files:
        full_name = os.path.join(root, fname)
        if full_name.endswith(".pdf") or full_name.endswith(".x"):
            #print "skipping :", full_name
            continue

        values=fname.split("_")
        values=map(lambda v: re.sub("[^0-9]", "",v), values)

        title=("Threads {}, Capacity {}, Prefill {}, Enqueue {}, Dequeue {}:"
                .format(*values))
        print "parsing:", full_name, title
        cmd = [
            'gnuplot',
            '-e', 'set title "%s"' % title,
            '-e', 'file="%s"' % full_name,
            'barplot.gp',
        ]
        p = sp.Popen(cmd, stderr=sp.STDOUT, stdout=sp.PIPE)
        p.wait()
        sys.stdout.write(p.communicate()[0])
