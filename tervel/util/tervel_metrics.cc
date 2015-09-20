#include <tervel/util/tervel_metrics.h>


namespace tervel {
namespace util {

constexpr const char* const EventTracker::event_code_strings[];
constexpr const char* const EventTracker::event_values_strings[];

void EventTracker::p_countEventOccurance(event_code_t code) {
  events_[static_cast<size_t>(code)]++;
}



void EventTracker::p_trackEventValue(event_values_code_t code, int64_t val) {
  event_values_[static_cast<size_t>(code)].update(val);
}

void EventTracker::add(EventTracker *other){
  for (size_t i = 0; i < static_cast<size_t>(event_code_t::END); i++) {
    events_[i] += other->events_[i];
  }
  for (size_t i = 0; i < static_cast<size_t>(event_values_code_t::END); i++) {
    event_values_[i].add(&(other->event_values_[i]));
  }
}

std::string EventTracker::generateYaml(int tid){

  std::string yaml_trace = "";

  if (tid != -1) {
      yaml_trace += "      - TID : " + std::to_string(tid) + "\n";
  }
  for (size_t i = 0; i< static_cast<size_t>(event_code_t::END); i++){
    yaml_trace += "        ";
    yaml_trace += event_code_strings[i];
    yaml_trace += " : ";
    yaml_trace += std::to_string(events_[i]);
    yaml_trace += "\n";
  }

  for (size_t i = 0; i< static_cast<size_t>(event_values_code_t::END); i++){
    yaml_trace += "        ";
    yaml_trace += event_values_strings[i];
    yaml_trace += " : ";
    yaml_trace += event_values_[i].yaml_string();
    yaml_trace += "\n";

  }

  return yaml_trace;
}


}
}
