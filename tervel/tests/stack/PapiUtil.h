
#include <vector>
#include <map>
#include <thread>
// #include <pthread.h>
#include <papi.h>

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

  std::vector<int> CacheDIT {PAPI_L1_TCA,PAPI_L1_TCH,PAPI_L1_TCM};


  std::vector<int> CacheInfo0 {PAPI_L1_TCA,PAPI_L1_DCA,PAPI_L1_ICA};//L1Access
  std::vector<int> CacheInfo1 {PAPI_L1_TCM,PAPI_L1_DCM,PAPI_L1_ICM};//L1 Miss
  std::vector<int> CacheInfo2 {PAPI_L2_TCA,PAPI_L2_DCA};//L2 Access
  std::vector<int> CacheInfo3 {PAPI_L2_ICA,PAPI_L2_ICM};//L2 AccessPAPI_TOT_INS
  std::vector<int> CacheInfo4 {PAPI_L2_TCM,PAPI_L2_DCM};//L2 Access
  std::vector<int> CacheInfo5 {PAPI_L1_ICR,PAPI_TOT_INS,PAPI_TOT_CYC};

  std::vector<int> CacheInfo6 {PAPI_L1_TCA,PAPI_L1_TCH,PAPI_L1_TCM};//L1 Total
  std::vector<int> CacheInfo7 {PAPI_L1_DCA,PAPI_L1_DCH,PAPI_L1_DCM};//L1 Total
  std::vector<int> CacheInfo8 {PAPI_L1_ICA,PAPI_L1_ICH,PAPI_L1_ICM};//L1 Total

  std::vector<int> CacheInfo9 {PAPI_L1_DCA,PAPI_L2_DCA,PAPI_L1_DCM};//L1 Total
  std::vector<int> CacheInfo10 {PAPI_TOT_INS,PAPI_TOT_CYC};//L1 Total
  // std::vector<int> CacheInfo8 {PAPI_L2_TCA,PAPI_L1_TCM};//L1 Total
  // std::vector<int> CacheInfo9 {PAPI_L2_ICA,PAPI_L1_ICM};//L1 Total

  std::vector<std::vector<int>> cacheInfo {CacheInfo0, CacheInfo1, CacheInfo2, CacheInfo3,CacheInfo4,CacheInfo5,CacheInfo6,CacheInfo7, CacheInfo8, CacheInfo9, CacheInfo10};

void printResults(std::vector<int> events, long long *counts);

std::vector<int> filterAvailableEvents(std::vector<int> events);

bool init(){
  int retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT) {
        printf("PAPI library init error!\n");
        return false;
    }
  retval = PAPI_thread_init( ( unsigned long ( * )( void ) )
           ( pthread_self ) );
  if ( retval != PAPI_OK ) {
     if ( retval == PAPI_ECMP ) {
        std::cout << "PAPI ERROR ! ( " << retval << " )" << std::endl;
        return false;
     }
     else {
        std::cout << "PAPI ERROR ! ( " << retval << " )" << std::endl;
        return false;
     }
  }
  return true;
}

bool start(std::vector<int> events, std::vector<int>& availableEvents){

  availableEvents = filterAvailableEvents(events);
  int res = PAPI_start_counters(&availableEvents[0],availableEvents.size());
  if(res != PAPI_OK){
    std::cout << "PAPI ERROR ! ( " << res << " )" << std::endl;
    return false;
  }
  return true;
}

void stop(long long *counts,int numOfEvents){
  int res = PAPI_stop_counters(counts, numOfEvents);
  if(res != PAPI_OK){
    std::cout << "PAPI ERROR(Stop Counters) ! ( " << res << " )" << std::endl;
    // return false;
  }
}

void shutdown(){
    PAPI_shutdown();
}


};

void PapiUtil::printResults(std::vector<int> events, long long *counts) {
    std::cout << "PAPI Results" << std::endl;
    for(unsigned int i = 0; i < events.size(); i++)
      std::cout << "\t" << PapiUtil::descriptionMap[events[i]] << ": " << counts[i] << std::endl;
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