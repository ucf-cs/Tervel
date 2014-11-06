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
