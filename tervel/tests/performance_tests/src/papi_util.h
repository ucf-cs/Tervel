/*
#The MIT License (MIT)
#
#Copyright (c) 2015 University of Central Florida's Computer Software Engineering
#Scalable & Secure Systems (CSE - S3) Lab
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.
#
*/

#ifndef __PAPIUTIL_H_
#define __PAPIUTIL_H_

#include <exception>
#include <cstring>
#include <vector>
#include <sstream>
#include <papi.h>

DEFINE_string(papi_events, "PAPI_L1_TCA,PAPI_L1_DCA,PAPI_L1_ICA","A comma separated list of events to track");


class PapiException : public std::exception{
  public:

    PapiException(std::string message){
      message_ = message;
    }

    virtual const char* what() const throw(){
      return message_.c_str();
    }
  private:
    std::string message_;
};

class PapiUtil {
  public:
    PapiUtil(std::string events=FLAGS_papi_events){
      try{
        int retval = PAPI_library_init(PAPI_VER_CURRENT);
        if (retval != PAPI_VER_CURRENT) {
          std::string msg = "PAPI library init error: " + std::to_string(retval);
          std::cout << msg << std::endl;
          throw new PapiException(msg);
        }
        init_event_set();
        init_options();
        addAllEvents(events);

        values_ = new long long[num_of_events_];
        for (int i = 0; i < num_of_events_; ++i){
          values_[i] = 0;
        }
      }catch(std::exception& e){
        std::cout << e.what() << std::endl;
      }
    }


    bool start(){
      int res = PAPI_start(event_set_);
      if(res != PAPI_OK){
        std::cout << "PAPI START ERROR ! ( " << res << " )" << std::endl;
        return false;
      }
      return true;
    }

    bool stop(){
      int retval = 0;
      if ( ( retval = PAPI_stop( event_set_, values_ ) ) != PAPI_OK ){
        std::cout << "PAPI STOP ERROR ! ( " << retval << " )" << std::endl;
        return false;
      }
      return true;
    }

    std::string results(){
      std::string res("  PAPIResults : \n");
      for(unsigned int i = 0; i < events_vector_.size(); i++){
        char event_name[PAPI_MAX_STR_LEN];
        PAPI_event_code_to_name(events_vector_[i],event_name);
        std::string event_str(event_name);
        res += "    " + event_str + " : " + std::to_string(values_[i])+ "\n";
      }
      return res;

    }

    void shutdown(){
        PAPI_shutdown();
    }
  private:
    long long *values_;
    PAPI_option_t opt_;
    int event_set_;
    int num_of_events_;
    std::vector<int> events_vector_;
    static const char EVENT_LIST_DELIMITER_ = ',';


    void addEvent(const char* eventName){
      int eventCode;
      int retval = PAPI_event_name_to_code( (char *)eventName, &eventCode );
      if(retval != PAPI_OK){
        std::string str(eventName);
        std::string msg = "PAPI event name(" + str + ") to code error: " + std::to_string(retval);
        std::cout << msg << std::endl;
        throw new PapiException(msg);
      }
      if ( ( retval = PAPI_query_event( eventCode ) ) != PAPI_OK ){
        std::string msg = "PAPI query event error: " + std::to_string(retval);
        std::cout << msg << std::endl;
        throw new PapiException(msg);
      }

      if ( ( retval = PAPI_add_event( event_set_, eventCode ) ) != PAPI_OK ){
        std::string str(eventName);
        std::string msg = "PAPI add event (" + str  + ") error " + std::to_string(retval);
        std::cout << msg << std::endl;
        throw new PapiException(msg);
      }
      num_of_events_++;
      events_vector_.push_back(eventCode);
    };

    void addAllEvents(std::string allEvents){
      num_of_events_ = 0;
      std::vector<std::string> events = split(allEvents,EVENT_LIST_DELIMITER_);
      for (unsigned int i = 0; i < events.size(); ++i){
        addEvent(events[i].c_str());
      }
    };



    void init_event_set(){
      int retval = 0;
      event_set_ = PAPI_NULL;
      if ( ( retval = PAPI_create_eventset( &event_set_ ) ) != PAPI_OK ){
        std::string msg = "PAPI create event_set error: " + retval;
        std::cout << msg << std::endl;
        throw new PapiException(msg);
      }

      if ( ( retval = PAPI_assign_eventset_component( event_set_, 0 ) ) != PAPI_OK ){
        std::string msg = "PAPI assign event_set component error: " + retval;
        std::cout << msg << std::endl;
        throw new PapiException(msg);
      }
    };

    void init_options(){

      int retval = 0;
      memset( &opt_, 0x0, sizeof ( PAPI_option_t ) );
      opt_.inherit.inherit = PAPI_INHERIT_ALL;
      opt_.inherit.eventset = event_set_;
      if ( ( retval = PAPI_set_opt( PAPI_INHERIT, &opt_ ) ) != PAPI_OK ) {
        if ( retval == PAPI_ECMP) {
          std::string msg = "PAPI Error! Inherit not supported by current component.";
          std::cout << msg << std::endl;
          throw new PapiException(msg);
        } else {
          std::string msg = "PAPI_set_opt error: " + retval;
          std::cout << msg << std::endl;
          throw new PapiException(msg);
        }
      }

    };

    std::vector<std::string> split(const std::string &s, char delim) {
      std::vector<std::string> elems;
      std::stringstream ss(s);
      std::string item;
      while (std::getline(ss, item, delim)) {
        std::stringstream trimmer;
        trimmer << item;
        item.clear();
        trimmer >> item;
        elems.push_back(item);
      }
      return elems;
    }




};
#endif  // __PAPIUTIL_H_