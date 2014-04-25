#ifndef TERVEL_MEMORY_OP_RECORD_H_
#define TERVEL_MEMORY_OP_RECORD_H_

#include "tervel/memory/hp/hp_element.h"

namespace tervel {
namespace thread {
  namespace HP { class HPElement; }
  class OpRecord: public HPElement {
   public:
    OpRecord() {}

    virtual void help_complete() = 0;
  };
}  // End Threading Name Space
}  // End Tervel Name Space
#endif  // TERVEL_MEMORY_OP_RECORD_H_
