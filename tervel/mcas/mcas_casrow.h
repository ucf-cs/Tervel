#ifndef TERVEL_MCAS_CASROW_H_
#define TERVEL_MCAS_CASROW_H_

#include "tervel/util/info.h"
#include "tervel/util/thread/util.h"
#include "tervel/mcas/mcas.h"
#include "tervel/mcas/mcas_helper.h"

#include <algorithm>

namespace tervel {
namespace mcas {

class Helper;
class MCAS;
/**
 * This class is used to represent a one of the M CAS operations performed
 * by an MCAS operation.
 * It holds an address, expected value and new value for that address.
 */
template<class T>
class CasRow {
  // REVIEW(carlos): this won't work like you expect it to.
  // RESPONSE(steven): you mean typedefing?
  typedef CasRow<T> t_CasRow;
  typedef Helper<T> t_Helper;
  typedef MCAS<T> t_MCAS;

  public:
    CasRow<T>() {}

    CasRow<T>(std::atomic<T> *address, T expected_value, T new_value)
        : address_ {a}
        , expected_value_ {ev}
        , new_value_ {nv} {}

    ~CasRow<T>() {}


    friend bool operator<(const t_CasRow& a, const t_CasRow& b) {
      return ((uintptr_t)a.address_) < ((uintptr_t)b.address_);
    };
    friend bool operator>(const t_CasRow& a, const t_CasRow& b) {
      return ((uintptr_t)a.address_) > ((uintptr_t)b.address_);
    };
    friend bool operator==(const t_CasRow& a, const t_CasRow& b) {
      return ((uintptr_t)a.address_) == ((uintptr_t)b.address_);
    };
    friend bool operator!=(const t_CasRow& a, const t_CasRow& b) {
      return ((uintptr_t)a.address_) != ((uintptr_t)b.address_);
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
      swap(a.expected_value_, b.expected_value_);
      swap(a.new_value_, b.new_value_);
      swap(a.address_, b.address_);
    };

    std::atomic<T> *address_;
    T expected_value_;
    T new_value_;
    std::atomic<t_Helper *>helper_;
};

}  // namespace mcas
}  // namespace tervel
#endif  // TERVEL_MCAS_CASROW_H_
