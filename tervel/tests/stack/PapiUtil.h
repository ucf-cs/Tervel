
#include <vector>
#include <map>
#include "papi.h"

class PapiUtil {
public:
  std::vector<int> CacheRequestsEvents {PAPI_CA_CLN,PAPI_CA_INV,PAPI_CA_ITV,PAPI_CA_SHR,PAPI_CA_SNP};
  std::vector<int> ConditionalStoreEvents {PAPI_CSR_FAL,PAPI_CSR_SUC,PAPI_CSR_TOT};
  std::vector<int> L1CacheAccessEvents {PAPI_L1_DCA,PAPI_L1_DCH,PAPI_L1_DCM,PAPI_L1_DCR,PAPI_L1_DCW,PAPI_L1_ICA,
                             PAPI_L1_ICH,PAPI_L1_ICM,PAPI_L1_ICR,PAPI_L1_ICW,PAPI_L1_LDM,PAPI_L1_STM,
                             PAPI_L1_TCA,PAPI_L1_TCH,PAPI_L1_TCM,PAPI_L1_TCR,PAPI_L1_TCW};
  std::vector<int> L2CacheAccessEvents {PAPI_L2_DCA,PAPI_L2_DCH,PAPI_L2_DCM,PAPI_L2_DCR,PAPI_L2_DCW,PAPI_L2_ICA,
                             PAPI_L2_ICH,PAPI_L2_ICM,PAPI_L2_ICR,PAPI_L2_ICW,PAPI_L2_LDM,PAPI_L2_STM,
                             PAPI_L2_TCA,PAPI_L2_TCH,PAPI_L2_TCM,PAPI_L2_TCR,PAPI_L2_TCW};
  std::vector<int> L3CacheAccessEvents {PAPI_L3_DCA,PAPI_L3_DCH,PAPI_L3_DCM,PAPI_L3_DCR,PAPI_L3_DCW,PAPI_L3_ICA,
                              PAPI_L3_ICH,PAPI_L3_ICM,PAPI_L3_ICR,PAPI_L3_ICW,PAPI_L3_LDM,PAPI_L3_STM,
                              PAPI_L3_TCA,PAPI_L3_TCH,PAPI_L3_TCM,PAPI_L3_TCR,PAPI_L3_TCW};
  std::vector<int> DataAccessEvents {PAPI_LD_INS,PAPI_LST_INS,PAPI_LSU_IDL,PAPI_MEM_RCY,PAPI_MEM_SCY,PAPI_MEM_WCY,
                              PAPI_PRF_DM,PAPI_RES_STL,PAPI_SR_INS, PAPI_STL_CCY,PAPI_STL_ICY,PAPI_SYC_INS};
  std::vector<int> TLBOperationsEvents {PAPI_TLB_DM,PAPI_TLB_IM,PAPI_TLB_SD,PAPI_TLB_TL};
  std::vector<int> CacheDataAccess {PAPI_L1_DCA,PAPI_L2_DCA,PAPI_L3_DCA};
  std::vector<int> CacheDataHit {PAPI_L1_DCH,PAPI_L2_DCH,PAPI_L3_DCH};
  std::vector<int> CacheDataMiss {PAPI_L1_DCM,PAPI_L2_DCM,PAPI_L3_DCM};
  std::vector<int> L1CacheDataAHM {PAPI_L1_DCA,PAPI_L1_DCH,PAPI_L1_DCM};
  std::vector<int> L2CacheDataAHM {PAPI_L2_DCA,PAPI_L2_DCH,PAPI_L2_DCM};
  std::vector<int> CacheDataAHM {PAPI_L1_DCA,PAPI_L2_DCA,PAPI_L2_DCM};

void printResults(std::vector<int> events, long long *counts);

std::vector<int> filterAvailableEvents(std::vector<int> events);

bool start(std::vector<int> events, std::vector<int>& availableEvents){
  int retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT) {
        printf("PAPI library init error!\n");
        return false;
    }
  availableEvents = filterAvailableEvents(events);
  int res = PAPI_start_counters(&availableEvents[0],availableEvents.size());
  if(res != PAPI_OK){
    std::cout << "PAPI ERROR ! ( " << res << " )" << std::endl;
    return false;
  }
  return true;
}

void stop(long long *counts,int numOfEvents){
  PAPI_stop_counters(counts, numOfEvents);
  PAPI_shutdown();
}

private:
std::map<int, std::string> descriptionMap = {
  {PAPI_L1_DCA, "L1 data cache accesses= "},
  {PAPI_L1_DCH, "L1 data cache hits= "},
  {PAPI_L1_DCM, "L1 data cache misses= "},
  {PAPI_L1_DCR, "L1 data cache reads= "},  
  {PAPI_L1_DCW, "L1 data cache writes= "},
  {PAPI_L1_ICA, "L1 instruction cache accesses= "},
  {PAPI_L1_ICH, "L1 instruction cache hits= "},
  {PAPI_L1_ICM, "L1 instruction cache misses= "},
  {PAPI_L1_ICR, "L1 instruction cache reads= "},
  {PAPI_L1_ICW, "L1 instruction cache writes= "},
  {PAPI_L1_LDM, "L1 load misses= "},
  {PAPI_L1_STM, "L1 store misses= "},  
  {PAPI_L1_TCA, "L1 total cache accesses= "},
  {PAPI_L1_TCH, "L1 total cache hits= "},
  {PAPI_L1_TCM, "L1 total cache misses= "},
  {PAPI_L1_TCR, "L1 total cache reads= "},
  {PAPI_L1_TCW, "L1 total cache writes= "},
  {PAPI_L2_DCA, "L2 data cache accesses= "},
  {PAPI_L2_DCH, "L2 data cache hits= "},
  {PAPI_L2_DCM, "L2 data cache misses= "},
  {PAPI_L2_DCR, "L2 data cache reads= "},  
  {PAPI_L2_DCW, "L2 data cache writes= "},
  {PAPI_L2_ICA, "L2 instruction cache accesses= "},
  {PAPI_L2_ICH, "L2 instruction cache hits= "},
  {PAPI_L2_ICM, "L2 instruction cache misses= "},
  {PAPI_L2_ICR, "L2 instruction cache reads= "},
  {PAPI_L2_ICW, "L2 instruction cache writes= "},
  {PAPI_L2_LDM, "L2 load misses= "},
  {PAPI_L2_STM, "L2 store misses= "},  
  {PAPI_L2_TCA, "L2 total cache accesses= "},
  {PAPI_L2_TCH, "L2 total cache hits= "},
  {PAPI_L2_TCM, "L2 total cache misses= "},
  {PAPI_L2_TCR, "L2 total cache reads= "},
  {PAPI_L2_TCW, "L2 total cache writes= "},
  {PAPI_L3_DCA, "L3 data cache accesses= "},
  {PAPI_L3_DCH, "L3 data cache hits= "},
  {PAPI_L3_DCM, "L3 data cache misses= "},
  {PAPI_L3_DCR, "L3 data cache reads= "},  
  {PAPI_L3_DCW, "L3 data cache writes= "},
  {PAPI_L3_ICA, "L3 instruction cache accesses= "},
  {PAPI_L3_ICH, "L3 instruction cache hits= "},
  {PAPI_L3_ICM, "L3 instruction cache misses= "},
  {PAPI_L3_ICR, "L3 instruction cache reads= "},
  {PAPI_L3_ICW, "L3 instruction cache writes= "},
  {PAPI_L3_LDM, "L3 load misses= "},
  {PAPI_L3_STM, "L3 store misses= "},  
  {PAPI_L3_TCA, "L3 total cache accesses= "},
  {PAPI_L3_TCH, "L3 total cache hits= "},
  {PAPI_L3_TCM, "L3 total cache misses= "},
  {PAPI_L3_TCR, "L3 total cache reads= "},
  {PAPI_L3_TCW, "L3 total cache writes= "},    

  {PAPI_LD_INS, "Load instructions= "},
  {PAPI_LST_INS, "Load/store instructions completed= "},
  {PAPI_LSU_IDL, "Cycles load/store units are idle= "},
  {PAPI_MEM_RCY, "Cycles Stalled Waiting for memory Reads= "},  
  {PAPI_MEM_SCY, "Cycles Stalled Waiting for memory accesses= "},
  {PAPI_MEM_WCY, "Cycles Stalled Waiting for memory writes= "},
  {PAPI_PRF_DM, "Data prefetch cache misses= "},
  {PAPI_RES_STL, "Cycles stalled on any resource= "},
  {PAPI_SR_INS, "Store instructions= "},
  {PAPI_STL_CCY, "Cycles with no instructions completed= "},
  {PAPI_STL_ICY, "Cycles with no instruction issue= "},
  {PAPI_SYC_INS, "Synchronization instructions completed= "}, 

  {PAPI_TLB_DM, "Data translation lookaside buffer misses= "},
  {PAPI_TLB_IM, "Instruction translation lookaside buffer misses= "},
  {PAPI_TLB_SD, "Translation lookaside buffer shootdowns= "},
  {PAPI_TLB_TL, "Total translation lookaside buffer misses= "},    

  {PAPI_CSR_FAL, "Failed store conditional instructions= "},
  {PAPI_CSR_SUC, "Successful store conditional instructions= "},
  {PAPI_CSR_TOT, "Total store conditional instructions= "},  

  {PAPI_CA_CLN, "Requests for exclusive access to clean cache line= "},
  {PAPI_CA_INV, "Requests for cache line invalidation= "},
  {PAPI_CA_ITV, "Requests for cache line intervention= "},
  {PAPI_CA_SHR, "Requests for exclusive access to shared cache line= "},
  {PAPI_CA_SNP, "Requests for a snoop= "}
};  
};

void PapiUtil::printResults(std::vector<int> events, long long *counts) {
    for(unsigned int i = 0; i < events.size(); i++)  
      std::cout << PapiUtil::descriptionMap[events[i]] << counts[i] << std::endl;
};

std::vector<int> PapiUtil::filterAvailableEvents(std::vector<int> events) {
  std::vector<int> availableEvents; 
  std::for_each(events.begin(), events.end(),
                [&availableEvents,this](int &e) {
                     if(PAPI_query_event(e) == PAPI_OK){
                      availableEvents.push_back(e);
                    }
                  });
  return availableEvents;
};