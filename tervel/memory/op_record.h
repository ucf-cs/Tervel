#ifndef TERVEL_MEMORY_OP_RECORD_H_
#define TERVEL_MEMORY_OP_RECORD_H_

#include "tervel/memory/hp/hp_element.h"

namespace tervel {
namespace memory {
// REVIEW(carlos): namespace declarations shouldn't add an extra indentation
//   level. Put a blank line after list of namespaces for readability.
// REVIEW(carlos): no such namespace RC (capitals)
  namespace HP { class HPElement; }
  // REVIEW(carlos): Should have space on both sides of extension colon
  // REVIEW(carlos): There's already a class called ::tervel::memory::OpRecord
  //   defined in progress_assurance.h. Which is the correct one?
  class OpRecord: public HPElement {
   public:
    OpRecord() {}

    virtual void help_complete() = 0;
  };
// REVIEW(carlos): put blank line before closing namespace brackets for
//   readability
// REVIEW(carlos): when putting an "end" comment on a closing bracket, use
//   what's on line that started the block (or an abbreviated version thereof).
//   In this case, these two should read:
//     }  // namespace memory
//     }  // namespace tervel
}  // End Threading Name Space
}  // End Tervel Name Space
// REVIEW(carlos): put a blank line before closing the include guard #endif (for
//   readability)
#endif  // TERVEL_MEMORY_OP_RECORD_H_
