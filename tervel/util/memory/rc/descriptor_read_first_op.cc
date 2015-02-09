#include "descriptor_util.h"
#include "descriptor_read_first_op.h"

void ReadFirstOp::help_complete() {
    void *aValue = value_.load();
    while (aValue == NULL) {
      void *cvalue = address_->load();

      if (tervel::util::memory::rc::is_descriptor_first(cvalue)) {
        tervel::util::Descriptor *descr = tervel::util::memory::rc::unmark_first(cvalue);
        if (tervel::util::memory::rc::watch(descr, address_, cvalue)) {
          cvalue = descr->get_logical_value();
          util::memory::rc::unwatch(descr);
          value_.compare_exchange_strong(aValue, cvalue);
          break;
        } else {
          aValue = value_.load();
          util::backoff();
        }
      } else {
        value_.compare_exchange_strong(aValue, cvalue);
        break;
      }
    }
  }

  void *ReadFirstOp::load() {
	  return value_.load();
  }
