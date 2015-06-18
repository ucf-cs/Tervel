#ifndef testerMacros_h_
#define testerMacros_h_

#include <gflags/gflags.h>

#define __TERVEL_MACRO_xstr(s) __TERVEL_MACRO_str(s)
#define __TERVEL_MACRO_str(s) #s

#define DS_EXTRA_END_SIGNAL

#define MACRO_OP_MAKER(opid, opcode) \
  if (op <= func_call_rate[ opid ]) { \
    opcode \
    \
    op_counter[ opid ].inc(opRes); \
    continue; \
  } \
  \

#endif // testerMacros_h_

