// REVIEW(carlos): should rename file to casrow.h
// REVIEW(carlos): should be TERVEL_*
#ifndef UCF_MCAS_CASROW_H_
#define UCF_MCAS_CASROW_H_

// REVIEW(carlos): put blank line between include block groups. Groups are:
//   1. .h file associated to current .cc file
//   2. c++ standard files
//   3. c standard files
//   4. external library files
//   5. internal library files
#include <algorithm>
// REVIEW(carlos): should be tervel/memory/*
#include "thread/info.h"
#include "thread/util.h"

// REVIEW(carlos): missing namespace declarations

template<class T>
class CasRow {
  // REVIEW(carlos): class comment should be above class name
  /**
   * This class is used to represent a one of the M CAS operations performed
   * by an MCAS operation.
   * It holds an address, expected value and new value for that address.
   */
  // REVIEW(carlos): this eon't work like you expect it to.
  // REVIEW(carlos): MCASHelper and MCAS are not forward declared anywhere and
  //   their headers aren't included
  typedef CasRow<T> t_CasRow;
  typedef MCASHelper<T> t_MCASHelper;
  typedef MCAS<T> t_MCAS;

  public:
    CasRow<T>() {}

    // REVIEW(carlos): parameter names are obtuse
    CasRow<T>(std::atomic<T> *a, T ev, T nv)
        : address {a}
        // REVIEW(carlos): comma should line up w/ colon
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

// REVIEW(carlos): should be TERVEL_*
#endif  // UCF_MCAS_CASROW_H_
