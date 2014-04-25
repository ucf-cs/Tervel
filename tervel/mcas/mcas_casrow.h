#ifndef UCF_MCAS_CASROW_H_
#define UCF_MCAS_CASROW_H_

#include <algorithm>
#include "thread/info.h"
#include "thread/util.h"

template<class T>
class CasRow {
  typedef CasRow<T> t_CasRow;
  typedef MCASHelper<T> t_MCASHelper;
  typedef MCAS<T> t_MCAS;

  public:
    /**
     * This class is used to represent a one of the M CAS operations performed
     * by an MCAS operation.
     * It holds an address, expected value and new value for that address.
     */
    CasRow<T>() {}

    CasRow<T>(std::atomic<T> *a, T ev, T nv)
        : address {a}
         , expected_value {ev}
         , new_value {nv} {}

    ~CasRow<T>() {}


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

    /**
     * This funciton is used when the MCAS operation is sorted to re-arrange
     * the CasRows so they are sorted in a decsending manner.
     * Sorting is important to prevent cyclic dependices between MCAS operations.
     * We choose descedning as a secondary bound on the number of MCAS operations.
     * That could interfer with this operation.
     */
    friend void swap(CasRow& a, CasRow& b) {
      using std::swap;
      swap(a.expected_value, b.expected_value);
      swap(a.new_value, b.new_value);
      swap(a.address, b.address);
    };

    std::atomic<T> *address;
    T expected_value;
    T new_value;
    std::atomic<t_MCASHelper *>helper;
};

#endif  // UCF_MCAS_CASROW_H_
