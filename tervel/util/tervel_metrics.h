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

#ifndef TERVEL_UTIL_TERVEL_METRICS_H_
#define TERVEL_UTIL_TERVEL_METRICS_H_
#include <tervel/util/util.h>
#include <tervel/util/info.h>

namespace tervel {
namespace util{

//for future use
class EventScope{
  uint64_t counter;
};

/**
* Start of Event tracker class
*/

/**
#include <tervel/util/tervel_metrics.h>
#if tervel_track<metric name> == tervel_track_enable
  TERVEL_METRIC(<metric name>);
#endif
 */
/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param  [description]
 * @return [description]
 */

#define TERVEL_METRIC(metric_name) {\
  if (tervel_track_##metric_name) {\
    util::EventTracker::countEvent(util::EventTracker::event_code::metric_name); \
  }\
}


class EventTracker{

public:
  #define tervel_track_enable true
  #define tervel_track_disable false

  #define tervel_track_announcement_count tervel_track_enable
  #define tervel_track_max_recur_depth_reached tervel_track_enable
  #define tervel_track_rc_watch_fail tervel_track_enable
  #define tervel_track_hp_watch_fail tervel_track_enable
  #define tervel_track_rc_remove_descr tervel_track_enable
  #define tervel_track_rc_offload tervel_track_enable
  #define tervel_track_helped_announcement tervel_track_enable

  enum class event_code : size_t {
    #if tervel_track_announcement_count == tervel_track_enable
    announcement_count,
    #endif
    #if tervel_track_helped_announcement == tervel_track_enable
    helped_announcement,
    #endif
    #if tervel_track_max_recur_depth_reached == tervel_track_enable
    max_recur_depth_reached,
    #endif
    #if tervel_track_rc_watch_fail == tervel_track_enable
    rc_watch_fail,
    #endif
    #if tervel_track_hp_watch_fail == tervel_track_enable
    hp_watch_fail,
    #endif
    #if tervel_track_rc_remove_descr == tervel_track_enable
    rc_remove_descr,
    #endif
    #if tervel_track_rc_offload == tervel_track_enable
    rc_offload,
    #endif
    END
  };

  static const constexpr char* const event_code_strings[] = {
    #if tervel_track_announcements_count == tervel_track_enable
    "announcement_count",
    #endif
    #if tervel_track_helped_announcement == tervel_track_enable
    "helped_announcement",
    #endif
    #if tervel_track_max_recur_depth_reached == tervel_track_enable
    "max_recur_depth_reached",
    #endif
    #if tervel_track_rc_watch_fail == tervel_track_enable
    "rc_watch_fail",
    #endif
    #if tervel_track_hp_watch_fail == tervel_track_enable
    "hp_watch_fail",
    #endif
    #if tervel_track_rc_remove_descr == tervel_track_enable
    "rc_remove_descr",
    #endif
    #if tervel_track_rc_offload == tervel_track_enable
    "rc_offload",
    #endif
    ""
  };

  std::string generateYaml();

  EventTracker()
  : events_(new uint64_t[static_cast<size_t>(event_code::END)]()) {}

  static void countEvent(EventTracker::event_code eventCode,
  EventTracker* tracker = tervel::tl_thread_info->get_event_tracker()) {
    tracker->pcountEvent(eventCode);
  };

  void pcountEvent(event_code eventCode);
  void add(EventTracker *other);

public:
  ~EventTracker(){}

  std::unique_ptr<uint64_t[]> events_;

};

/**
* End of the event tracker class
*/

}  //  tervel    namespace
}  //  util   namespace

#endif // TERVEL_UTIL_TERVEL_METRICS_H_