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
#ifndef TERVEL_CONTAINERS_WF_STACK_STACK_OP_H_
#define TERVEL_CONTAINERS_WF_STACK_STACK_OP_H_

#include <tervel/util/util.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

#include <tervel/containers/wf/stack/stack.h>
#include <tervel/containers/wf/stack/helper.h>

namespace tervel {
namespace containers {
namespace wf {


template<typename T>
class Stack<T>::StackOp : public util::OpRecord {
 public:
  StackOp(Stack<T> *stack) : stack_(stack) { };
  ~StackOp() {
    Helper *h = helper_.load();
    assert(h != nullptr);
    if (h != fail_val_) {
      delete h;
    }
  };

  virtual bool associate(Helper *h) {
    Helper *temp = nullptr;
    bool res = helper_.compare_exchange_strong(temp, h);
    if (res || temp == nullptr) { // success
      return true;
    } else { // fail
      return false;
    }
  };

  void fail() {
    Helper *temp = nullptr;
    helper_.compare_exchange_strong(temp, fail_val_);
  };

  bool result(T &val) {
    Helper *helper = helper_.load();
    if (helper == fail_val_) {
      return false;
    } else {
      Node *elem = helper->old_value_;
      val = elem->value();
      return true;
    }

  }

  bool notValid(Helper * h) {
    return helper_.load() != h;
  }

  bool notDone() {
    return helper_.load() == nullptr;
  };

  bool on_watch(std::atomic<void *> *address, void *expected) {
    return true;
  };

  bool on_is_watched() {
    Helper *h = helper_.load();
    assert(h != nullptr);
    if (h != fail_val_) {
      bool res = tervel::util::memory::hp::HazardPointer::is_watched(h);
      return res;
    }
    return false;
  }

  static constexpr Helper * fail_val_ = reinterpret_cast<Helper *>(0x1L);

  Stack<T> * stack_;
  std::atomic<Helper *> helper_{nullptr};
  DISALLOW_COPY_AND_ASSIGN(StackOp);

};  // class StackOp<T>::StackOp


template<typename T>
class Stack<T>::PopOp: public StackOp {
 public:
  PopOp(Stack<T> *s)
    : StackOp(s) {}

  void help_complete() {
    Helper * helper = new Helper(this);
    Node *helper_marked = reinterpret_cast<Node *>(tervel::util::set_1st_lsb_1<Helper>(helper));

    while (StackOp::notDone()) {
      Accessor access;
      if (access.load(&(StackOp::stack_->lst_)) == false) {
        continue;
      };

      Node *cur = access.ptr();
      Node *next = nullptr;
      if (cur != nullptr) {
        next = cur->next();
      }

      helper->new_value_ = next;
      helper->old_value_ = cur;

      if (cur == nullptr) {
        StackOp::fail();
        break;
      } else if (StackOp::stack_->lst_.compare_exchange_strong(cur, helper_marked)) {
        helper->finish(&(StackOp::stack_->lst_), helper_marked);
        assert(StackOp::stack_->lst_.load() != helper_marked);
        if (StackOp::notValid(helper)) {
          helper->safe_delete();
        }
        return;
      }
    }  // while (true)
    delete helper;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(PopOp);
};  // PopOp


template<typename T>
class Stack<T>::PushOp: public StackOp {
 public:
  PushOp(Stack<T> *s, Node * elem)
    : StackOp(s)
    , elem_(elem)
     {
      elem_->atomic_next(reinterpret_cast<Node *>(this));
     }

  bool associate(Helper *h) {
    bool res = StackOp::associate(h);
    if (res) {
      //TODO(steven) Should we watch elem_?
      elem_->atomic_next(reinterpret_cast<Node *>(this), h->old_value_);
    }
    return res;
  }
  void help_complete() {
    Helper * helper = new Helper(this);
    helper->new_value_ = elem_;
    Node *helper_marked =  reinterpret_cast<Node *>(tervel::util::set_1st_lsb_1<Helper>(helper));

    while (StackOp::notDone()) {
      Accessor access;
      if (access.load(&(StackOp::stack_->lst_)) == false) {
        continue;
      };

      Node *cur = access.ptr();
      helper->old_value_ = cur;

      if (StackOp::stack_->lst_.compare_exchange_strong(cur, helper_marked)) {
        helper->finish(&(StackOp::stack_->lst_), helper_marked);
        assert(StackOp::stack_->lst_.load() != helper_marked);
        if (StackOp::notValid(helper)) {
          helper->safe_delete();
        }
        return;
      }
    }  // while (true)
    delete helper;
  }

 private:
  Node * const elem_;
  DISALLOW_COPY_AND_ASSIGN(PushOp);
};  // PushOp

}  // namespace WF
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_STACK_STACK_OP_H_
