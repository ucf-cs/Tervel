//
//  Created by Steven Feldman on 4/11/13.
//  Copyright (c) 2013 Steven FELDMAN. All rights reserved.
//

#ifndef WF_MCAS_3_HPP_
#define WF_MCAS_3_HPP_

#include "ucf_threading.h"
#include <algorithm>

namespace ucf {
namespace mcas {

  template<class T, int words>
  class CasRow;

  template<class T, int words>
  class MCASHelper;

  template<class T, int words>
  class MCAS;

  typedef thread::rc::Descriptor RCDescr;

  template<class T>
  T read(std::atomic<T> * address) {
    // TODO(steven) make wait-free
    T current = address->load();
    while (RCDescr::isDescr(current)) {
      current = RCDescr::remove(current, address);
    }
    return current;
  }


  template<class T, int words>
  class MCASHelper: public RCDescr {
    typedef CasRow<T, words> t_CasRow;
    typedef MCASHelper<T, words> t_MCASHelper;
    typedef MCAS<T, words> t_MCAS;

  public:
    t_CasRow * cr;
    t_CasRow *lastRow;

    MCASHelper<T, words>(t_CasRow *c, t_CasRow *l) {
      cr = c;
      lastRow = l;
    };

    bool advanceWatch(std::atomic<void *> *address, T value) {
      /*this->rc_count.fetch_add(1);  // This has been moved to watch() in rc::PoolElem
      if(address->load() != value) {
        this->rc_count.fetch_add(-1);
        return false;
      }*/

      int hpPos = thread::hp::ID::id_mcas;
      thread::hp::PoolElem::watch(reinterpret_cast<void *>(lastRow), hpPos);
      if (address->load() != value) {
        thread::hp::PoolElem::unwatch(hpPos);
        return false;
      }
      // Alright, now both this helper can not be changed
      // and the mcas it reference can not be changed.

      t_MCASHelper *temp = cr->helper.load();
      if (temp == nullptr) {
         if (cr->helper.compare_exchange_strong(temp, this)) {
           temp = this;
         }
      }


      if (temp == this) {
        // We have a hp and rc protection on the mcas because it is associated
        // but only need rc protecction.
        thread::hp::PoolElem::unwatch(hpPos);
        return true;
      } else {
        // We have a hp proctection only, because it is not associated!
          // --ie placed in error, op is already done...
          // Remove it!
        T temp2 = RCDescr::mark<T>(this);
        address->compare_exchange_strong(temp2, cr->expectedValue);
        thread::hp::PoolElem::unwatch(hpPos);
        return false;
      }
    };

    void unsafeFree() {
      thread::rc::PoolElem::addToSafe(this);
    }


    void safeFree() {
      if (thread::rc::PoolElem::isWatched(this)) {
        thread::rc::PoolElem::addToUnSafe(this);
      } else {
        unsafeFree();
      }
    }

    void * descr_complete(void * v, std::atomic<void *> *address) {
      t_MCASHelper* temp_null = nullptr;
      this->cr->helper.compare_exchange_strong(temp_null, this);

      bool success = false;
      if (temp_null == nullptr || temp_null == this) {
        assert(this->cr->helper.load() != nullptr);
        assert(this->cr->helper.load() != t_MCAS::MCAS_FAIL_CONST);
        success = t_MCAS::complete(this->cr, this->lastRow);
        if (thread::rReturn) {
          return nullptr;
        }
        assert(this->lastRow->helper.load() !=  nullptr);
      }

      if (success) {
        assert(this->lastRow->helper.load() != t_MCAS::MCAS_FAIL_CONST);
        address->compare_exchange_strong(v,
            reinterpret_cast<void *>(this->cr->newValue));
      } else {
        address->compare_exchange_strong(v,
            reinterpret_cast<void *>(this->cr->expectedValue));
      }
      return address->load();
    }

    static T mcasRemove(T t, std::atomic<T> *address,
                          t_CasRow *lastRow) {
      RCDescr *descr = Descriptor::unmark(t);
      bool watched = thread::rc::PoolElem::watch(descr,
                    reinterpret_cast<std::atomic<void *>*>(address),
                    reinterpret_cast<void *>(t));

      T newValue;
      if (watched) {
        t_MCASHelper* cast_p = static_cast<t_MCASHelper *>(descr);
        // TODO(steven): check to make sure this is safe...
        if ( (cast_p !=  nullptr) && (cast_p->lastRow == lastRow) ) {
          assert((uintptr_t)cast_p == (uintptr_t)descr);

          t_MCASHelper* temp_null = nullptr;
          cast_p->cr->helper.compare_exchange_strong(temp_null, cast_p);
          if (temp_null != nullptr && temp_null != cast_p) {
            address->compare_exchange_strong(t, cast_p->cr->expectedValue);
          }
          newValue = address->load();
        } else {
          newValue = RCDescr::remove(t, address);
        }
        thread::rc::PoolElem::unwatch(descr);
      } else {
        newValue = address->load();
      }

      return newValue;
    };
  };

  template<class T, int words>
  class CasRow: public thread::OpRecord {
    typedef CasRow<T, words> t_CasRow;
    typedef MCASHelper<T, words> t_MCASHelper;
    typedef MCAS<T, words> t_MCAS;

  public:
    std::atomic<T> *address;
    T expectedValue;
    T newValue;
    std::atomic<t_MCASHelper *>helper;

    CasRow<T, words>() {}

    CasRow<T, words>(std::atomic<T> *a, T ev, T nv) {
      address = a;
      expectedValue = ev;
      newValue = nv;
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
      swap(a.expectedValue, b.expectedValue);
      swap(a.newValue, b.newValue);
      swap(a.address, b.address);
    };

    bool wfcomplete(t_CasRow *lastRow) {  // must be lastRow
      thread::progressAssurance->askForHelp(lastRow);
      assert(lastRow->helper.load());
      return (lastRow->helper.load() != t_MCAS::MCAS_FAIL_CONST);
    }

    void helpComplete() {
      // Lets get to the start of this operation
      t_CasRow* current = (this - words)+1;
      t_MCAS::complete(current, this, true);  // Must start -1
    };

    bool advanceIsWatched() {
      // First HP check then loop check through each helper
      // t_CasRow *lastRow = &(crows[row_count-1];
      // bool res= thread::hp::PoolElem::isWatched(this, lastRow);
      // if (res) {
      //  return true;
      // }

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

    void unsafeFree() {
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
          mch->unsafeFree();
        }
        current++;
      }

      current = (this - (words - 1) );
      delete[] current;
    }

    void safeFree() {
      if (thread::hp::PoolElem::isWatched(this)) {
        // If true then it needs to be added to the dangerous linked list
        thread::hp::PoolElem::addToUnSafe(this);
      } else {
        // We can unsafe free it.
        unsafeFree();
      }
    }
  };


  template<class T, int words>
  class MCAS {
    typedef CasRow<T, words> t_CasRow;
    typedef MCASHelper<T, words> t_MCASHelper;
    typedef MCAS<T, words> t_MCAS;

  public:
    static constexpr t_MCASHelper * MCAS_FAIL_CONST =
                 reinterpret_cast<t_MCASHelper *>(0x1);
    t_CasRow *crows;

    int row_count;
    MCAS<T, words>() {
      row_count = 0;
      crows = new t_CasRow[words];
    };

    ~MCAS<T, words>() {
      if (row_count == words) {
        t_CasRow *lastRow = &(crows[row_count-1]);
        lastRow->safeFree();
        // Do not delete crows, this will be done in safeFree
      } else {
        assert(false);
        delete[] crows;
      }
    }

    bool addCASTriple(std::atomic<T> *a, T ev, T nv) {
      if (t_MCASHelper::isDescr(ev) ||  t_MCASHelper::isDescr(nv)) {
        return false;
      } else if (row_count == words) {
        return false;
      } else {
        crows[row_count].address = a;
        crows[row_count].expectedValue = ev;
        crows[row_count].newValue = nv;
        crows[row_count++].helper.store(nullptr);

        for (int i = (row_count - 1); i > 0; i--) {
          if (crows[i] > crows[i-1]) {
            swap(crows[i], crows[i-1]);
          } else if (crows[i] == crows[i-1]) {
            for (i = i; i < row_count-1; i++) {
              swap(crows[i], crows[i+1]);
            }
            row_count--;
            crows[row_count].address = reinterpret_cast<T>(nullptr);
            crows[row_count].expectedValue = reinterpret_cast<T>(nullptr);
            crows[row_count].newValue = reinterpret_cast<T>(nullptr);
            return false;
          }
        }
        return true;
      }
    };

    bool execute() {
      thread::progressAssurance->tryToHelp();

      if (row_count != words) {
        // TODO(steven): fix code to address when row_count < words
        // Also check if duplicate address...
        assert(false);
        return false;
      }

      bool res = complete(&(crows[0]), &(crows[row_count-1]));

      // Now Clean up
      cleanup(res, &(crows[0]), &(crows[row_count-1]));
      return res;
    };

    static bool complete(t_CasRow *current, t_CasRow *lastRow,
                                          bool wfmode = false) {
      current--;
      while (current != lastRow) {
        current++;
        T temp = current->address->load();

        size_t fcount = 0;

        while (current->helper.load() == nullptr) {
          if (!wfmode && fcount++ == thread::OpRecord::MAX_FAILURE) {
            if (thread::rDepth == 0) {
              // Make An annoucnement
              return current->wfcomplete(lastRow);
            } else {
              thread::rReturn = true;
              return false;
            }
          }  // End If Fail Count has been Reached

          if (RCDescr::isDescr(temp)) {  // Not should be replaced by isDescr?
            // Remove it by completing the op, try again
            temp = t_MCASHelper::mcasRemove(temp, current->address, lastRow);

            if (thread::rReturn) {
              if (thread::rDepth == 0) {
                thread::rReturn = false;
                temp = current->address->load();
              } else {
                return false;
              }
            }
            continue;
          } else if (temp != current->expectedValue) {
            t_MCASHelper* temp_n = nullptr;
            if (current->helper.compare_exchange_strong(temp_n, MCAS_FAIL_CONST)
              || temp_n == MCAS_FAIL_CONST) {
              temp_n = nullptr;
              lastRow->helper.compare_exchange_strong(temp_n, MCAS_FAIL_CONST);
              assert(lastRow->helper.load() == MCAS_FAIL_CONST);
              return false;
            }
            continue;
          }  else {
            t_MCASHelper *helper = new t_MCASHelper(current, lastRow);
            if (current->address->compare_exchange_strong(temp,
                                              RCDescr::mark<T>(helper))) {
              // Succesfully placed
              t_MCASHelper * temp_null = nullptr;
              if (current->helper.compare_exchange_strong(temp_null, helper)
                                                    || temp_null == helper) {
                // Succesfully associoated!
              } else {
                // Failed...op must be done!
                temp = RCDescr::mark<T>(helper);
                current->address->compare_exchange_strong(temp,
                                          current->expectedValue);
                helper->safeFree();
              }
              break;
            } else {
              // Failed to place...try again!
              // temp already holds the new value
              helper->unsafeFree();
              continue;
            }
          }  // End Else Try to replace
        }  // End loop on CasRow

        assert(current->helper.load() != nullptr);
        if (current->helper.load() == MCAS_FAIL_CONST) {
          t_MCASHelper* temp_n = nullptr;
          lastRow->helper.compare_exchange_strong(temp_n, MCAS_FAIL_CONST);
          assert(lastRow->helper.load() == MCAS_FAIL_CONST);
          return false;
        }
      }  // End While current != lastRow

      assert(lastRow->helper.load() != nullptr);
      return (lastRow->helper.load() != MCAS_FAIL_CONST);
    }  // End Complete function.

    static void cleanup(bool success, t_CasRow *current, t_CasRow *lastRow) {
      current--;
      while (current != lastRow) {
        current++;

        assert(current->helper.load() != nullptr);
        T hTemp = RCDescr::mark<T>(current->helper.load());
        if (hTemp == reinterpret_cast<T>(MCAS_FAIL_CONST)) {
          return;
        }

        T temp = current->address->load();

        if (temp == hTemp) {
          if (success) {
            current->address->compare_exchange_strong(temp, current->newValue);
          } else {
            current->address->compare_exchange_strong(temp,
                                      current->expectedValue);
          }
        }
      }  // End While loop over casrows.
    }  // End cleanup function.
  };
};  // namespace mcas
};  // namespace ucf

#endif  // WF_MCAS_3_HPP_
