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

// Struct used to track average values of a variable.
typedef struct event_values_t{
  float mean;
  float variance;
  float card;

  void operator()() {
    mean = 0;
    variance = 0;
    card = 0;
  }

  void update(int64_t value) {
    mean =  ((mean * card) + value) / (card + 1.0);
    float diff = value - mean;
    variance = ((variance * (card * card)) + (diff * diff));
    card += 1.0;
    variance = variance / (card * card);
  }

  void add(struct event_values_t *other) {
    mean += other->mean;
    variance += other->variance;
    card += other->card;
  }

  std::string yaml_string() {
    std::string str = "";
    str += "\n          mean : " + std::to_string(mean);
    str += "\n          variance : " + std::to_string(variance);
    str += "\n          card : " + std::to_string(card);
    return str;
  }
}event_values_t;

/**
* Start of Event tracker class
*/

/**
#include <tervel/util/tervel_metrics.h>
#if tervel_track<metric name> == tervel_track_enable
  TERVEL_METRIC(<metric name>);
#endif
 */


#ifdef USE_TERVEL_METRICS
#define TERVEL_METRIC(metric_name) {\
  if (tervel_track_##metric_name) {\
    util::EventTracker::countEvent(util::EventTracker::event_code_t::metric_name); \
  }\
}


#define TERVEL_METRIC_TRACK_VALUE(metric_name, value) {\
  if (tervel_track_##metric_name) {\
    util::EventTracker::trackEventValue(util::EventTracker::event_values_code_t::metric_name, value); \
  }\
}
#else
  #define TERVEL_METRIC(metric_name)
  #define TERVEL_METRIC_TRACK_VALUE(metric_name, value)
#endif


class EventTracker{

public:
  #define tervel_track_enable true
  #define tervel_track_disable false

  #define tervel_track_limit_value tervel_track_enable
  #define tervel_track_announcement_count tervel_track_enable
  #define tervel_track_max_recur_depth_reached tervel_track_enable
  #define tervel_track_rc_watch_fail tervel_track_enable
  #define tervel_track_hp_watch_fail tervel_track_enable
  #define tervel_track_rc_remove_descr tervel_track_enable
  #define tervel_track_rc_is_descr tervel_track_enable
  #define tervel_track_rc_offload tervel_track_enable
  #define tervel_track_helped_announcement tervel_track_enable
  #define tervel_track_is_delayed_count tervel_track_enable


  enum class event_code_t : size_t {
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
    #if tervel_track_rc_is_descr == tervel_track_enable
    rc_is_descr,
    #endif
    #if tervel_track_rc_offload == tervel_track_enable
    rc_offload,
    #endif
    #if tervel_track_is_delayed_count == tervel_track_enable
    is_delayed_count,
    #endif
    END
  };

  static const constexpr char* const event_code_strings[] = {
    #if tervel_track_announcement_count == tervel_track_enable
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
    #if tervel_track_rc_is_descr == tervel_track_enable
    "rc_is_descr",
    #endif
    #if tervel_track_rc_offload == tervel_track_enable
    "rc_offload",
    #endif
    #if tervel_track_is_delayed_count == tervel_track_enable
    "is_delayed_count",
    #endif
    ""
  };

  enum class event_values_code_t : size_t {
    #if tervel_track_limit_value == tervel_track_enable
    limit_value,
    #endif
    END
  };

  static const constexpr char* const event_values_strings[] = {
    #if tervel_track_limit_value == tervel_track_enable
    "limit_value",
    #endif
    ""
  };


  std::string generateYaml(int tid = -1);

  EventTracker()
  : events_(new uint64_t[static_cast<size_t>(event_code_t::END)]())
  , event_values_(new event_values_t[static_cast<size_t>(event_values_code_t::END)]())
  {}

  static void countEvent(EventTracker::event_code_t code,
  EventTracker* tracker = tervel::tl_thread_info->get_event_tracker()) {
    tracker->p_countEventOccurance(code);
  };

  static void trackEventValue(EventTracker::event_values_code_t code, int64_t val,
  EventTracker* tracker = tervel::tl_thread_info->get_event_tracker()) {
    tracker->p_trackEventValue(code, val);
  };

  void p_countEventOccurance(event_code_t code);
  void p_trackEventValue(event_values_code_t code, int64_t val);
  void add(EventTracker *other);

public:
  ~EventTracker(){}

  std::unique_ptr<uint64_t[]> events_;
  std::unique_ptr<event_values_t[]> event_values_;

};

/**
* End of the event tracker class
*/

}  //  tervel    namespace
}  //  util   namespace

#endif // TERVEL_UTIL_TERVEL_METRICS_H_