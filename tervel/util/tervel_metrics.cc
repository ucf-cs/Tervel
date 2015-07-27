#include <tervel/util/tervel_metrics.h>


namespace tervel {
namespace util {

constexpr const char* const EventTracker::event_code_strings[];

void EventTracker::pcountEvent(event_code eventCode) {
  events_[static_cast<size_t>(eventCode)]++;
}

void EventTracker::add(EventTracker *other){
  for (size_t iterator = 0; iterator < static_cast<size_t>(event_code::END); iterator++) {
    events_[iterator] += other->events_[iterator];
  }
}

std::string EventTracker::generateYaml(){

  std::string yaml_trace = "  TERVELMETRICS:\n";

  for (size_t iterator = 0; iterator< static_cast<size_t>(event_code::END); iterator++){
    yaml_trace += "    ";
    yaml_trace += event_code_strings[iterator];
    yaml_trace += " : ";
    yaml_trace += std::to_string(events_[iterator]);
    yaml_trace += "\n";
  }

  return yaml_trace;
}


}
}
