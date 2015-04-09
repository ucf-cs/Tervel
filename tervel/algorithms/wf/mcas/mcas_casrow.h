/*
The MIT License (MIT)

Copyright (c) 2015 University of Central Florida's Computer Software Engineering
Scalable & Secure Systems (CSE - S3) Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef TERVEL_MCAS_CASROW_H_
#define TERVEL_MCAS_CASROW_H_

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/algorithms/wf/mcas/mcas_helper.h>
#include <algorithm>


namespace tervel {
namespace algorithms {
namespace wf {
namespace mcas {

template<class T>
class Helper;

/**
 * This class is used to represent a one of the M CAS operations performed
 * by an MCAS operation.
 * It holds an address, expected value and new value for that address.
 */
template<class T>
class CasRow {
  public:
    CasRow<T>() {}

    CasRow<T>(std::atomic<T> *address, T expected_value, T new_value)
        : address_ {address}
        , expected_value_ {expected_value}
        , new_value_ {new_value} {}

    ~CasRow<T>() {}


    friend bool operator<(const CasRow<T>& a, const CasRow<T>& b) {
      return ((uintptr_t)a.address_) < ((uintptr_t)b.address_);
    };
    friend bool operator>(const CasRow<T>& a, const CasRow<T>& b) {
      return ((uintptr_t)a.address_) > ((uintptr_t)b.address_);
    };
    friend bool operator==(const CasRow<T>& a, const CasRow<T>& b) {
      return ((uintptr_t)a.address_) == ((uintptr_t)b.address_);
    };
    friend bool operator!=(const CasRow<T>& a, const CasRow<T>& b) {
      return ((uintptr_t)a.address_) != ((uintptr_t)b.address_);
    };

    /**
     * This funciton is used when the MCAS operation is sorted to re-arrange
     * the CasRows so they are sorted in a decsending manner.
     * Sorting is important to prevent cyclic dependices between MCAS
     * operations. We choose descedning as a secondary bound on the number of
     * MCAS operations. That could interfer with this operation.
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
    std::atomic<Helper<T> *>helper_;
};  // Class CasRow

}  // namespace mcas
}  // namespace wf
}  // namespace algorithms
}  // namespace tervel
#endif  // TERVEL_MCAS_CASROW_H_
