#!/usr/bin/env python

import gdb

class SkiplistPrintCommand(gdb.Command):
    """Iterate and print a list.

skip <EXPR> [MAX]

Given a list EXPR, iterate though the list nodes' ->next pointers, printing
each node iterated. We will iterate thorugh MAX list nodes, to prevent
infinite loops with corrupt lists. If MAX is zero, we will iterate the
entire list.

List nodes types are expected to have a member named "next". List types
may be the same as node types, or a separate type with an explicit
head node, called "head"."""

    MAX_ITER = 10

    def __init__(self):
        super(SkiplistPrintCommand, self).__init__("skiplist-print", gdb.COMMAND_DATA, gdb.COMPLETE_SYMBOL)

    def invoke(self, _args, from_tty):
        args = gdb.string_to_argv(_args)
        start_node = args[0]

        if len(args) > 1:
            max_iter = int(args[1])
        else:
            max_iter = self.MAX_ITER

        if len(args) > 2:
            lvl = int(args[2])
        else:
            lvl = 0
        
        p_node_t = gdb.lookup_type('node_t').pointer()
        long_t = gdb.lookup_type('long')
        node = gdb.parse_and_eval(start_node)
        print node

        for i in xrange(max_iter):
            nexts = node['next']
            nxt = gdb.Value(nexts[lvl]).cast(long_t)
            nxt = nxt & ~1
            node = gdb.Value(nxt).cast(p_node_t).dereference()
            nexts = node['next']
            print node['k'], node['level'], node['inserting'],
            k = 0
            while k < node['level']:
                print(nexts[k]),
                k+=1
            print("")

SkiplistPrintCommand()
