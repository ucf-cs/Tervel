#ifndef testerMacros_h_
#define testerMacros_h_


#define MACRO_OP_MAKER(opid, opcode) \
  if (op <= func_call_rate[ opid ]) { \
    opcode \
    \
    op_counter[ opid ].inc(opRes); \
    continue; \
  } \
  \

#endif // testerMacros_h_

