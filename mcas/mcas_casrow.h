#ifndef UCF_MCAS_CASROW_H_
#define UCF_MCAS_CASROW_H_

#include <algorithm>
#include "thread/info.h"
#include "thread/util.h"

template<class T>
class CasRow: public thread::OpRecord {
  typedef CasRow<T> t_CasRow;
  typedef MCASHelper<T> t_MCASHelper;
  typedef MCAS<T> t_MCAS;

  public:
    std::atomic<T> *address;
    T expected_value;
    T new_value;
    std::atomic<t_MCASHelper *>helper;

    CasRow<T>() {}

    CasRow<T>(std::atomic<T> *a, T ev, T nv) {
      address = a;
      expected_value = ev;
      new_value = nv;
    };

    ~CasRow<T>() {
      t_CasRow* current = (this - (words - 1) );
      int i;
      for (i = 0; i < words; i++) {
        t_MCASHelper * mch = current->helper.load();
        if (mch == t_MCAS::MCAS_FAIL_CONST) {
          break;
        } else if (mch == nullptr) {
          assert(i == 0);
          break;
        } else {
          thread::rc::descriptor_pool::free_descriptor(mch, true);
        }
        current++;
      }

      current = (this - (words - 1) );
      delete[] current;
    };


    friend bool operator<(const t_CasRow& a, const t_CasRow& b) {
      return ((uintptr_t)a.address) < ((uintptr_t)b.address);
    };
    friend bool operator>(const t_CasRow& a, const t_CasRow& b) {
      return ((uintptr_t)a.address) > ((uintptr_t)b.address);
    };
    friend bool operator==(const t_CasRow& a, const t_CasRow& b) {
      return ((uintptr_t)a.address) == ((uintptr_t)b.address);
    };
    friend bool operator!=(const t_CasRow& a, const t_CasRow& b) {
      return ((uintptr_t)a.address) != ((uintptr_t)b.address);
    };

    friend void swap(CasRow& a, CasRow& b) {
      using std::swap;
      swap(a.expected_value, b.expected_value);
      swap(a.new_value, b.new_value);
      swap(a.address, b.address);
    };

    bool wf_complete(t_CasRow *last_row) {  // must be last_row
      tl_thread_info.progress_assurance->askForHelp(last_row);
      assert(last_row->helper.load());
      return (last_row->helper.load() != t_MCAS::MCAS_FAIL_CONST);
    }

    void help_complete() {
      // Lets get to the start of this operation
      t_CasRow* current = (this - words)+1;
      t_MCAS::mcas_complete(current, this, true);
    };

    bool advance_is_watched() {
      t_CasRow* current = (this - (words - 1) );
      int i;
      for (i = 0; i < words; i++) {
        t_MCASHelper * mch = current->helper.load();
        if (mch == t_MCAS::MCAS_FAIL_CONST) {
          break;
        } else if (mch == nullptr) {
          assert(i == 0);
          break;
        } else if (thread::rc::PoolElem::isWatched(mch)) {
          return true;
        }
        current++;
      }  // End For Loop over Rows
      return false;
    }
};

#endif  // UCF_MCAS_CASROW_H_
