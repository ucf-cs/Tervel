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
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hazard_pointer.h>


namespace tervel {
namespace util {

void ProgressAssurance::p_check_for_announcement(int64_t &help_id) {
    help_id++;
    if (help_id >= num_threads_) {
      help_id = 0;
    }

    OpRecord *op = op_table_[help_id].load();
    if (op != nullptr) {
      std::atomic<void *> *address = reinterpret_cast<std::atomic<void *> *>(
              &(op_table_[help_id]));

      typedef memory::hp::HazardPointer::SlotID SlotID;
      SlotID pos = SlotID::PROG_ASSUR;
      bool res = memory::hp::HazardPointer::watch(pos, op, address, op);
      if (res) {
        assert(memory::hp::HazardPointer::is_watched(op));
        op->help_complete();
        #ifdef tervel_track_helped_announcement
        util::EventTracker::countEvent(util::EventTracker::event_code::helped_announcement);
        #endif
        memory::hp::HazardPointer::unwatch(pos);
      }
    }
}

void ProgressAssurance::p_make_announcement(OpRecord *op, const uint64_t tid) {
  op_table_[tid].store(op);
  op->help_complete();
  op_table_[tid].store(nullptr);
}

}  // namespace memory
}  // namespace tervel
