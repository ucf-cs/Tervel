#ifndef UCF_MCAS_HELPER_
#define UCF_MCAS_HELPER_

#include "thread/info.h"
#include "mcas_casrow.h"
#include "mcas.h"
#include "thread/descriptor.h"



namespace ucf {
namespace mcas {

template<class T>
class MCASHelper: public thread::Descriptor {
typedef CasRow<T> t_CasRow;
typedef MCASHelper<T> t_MCASHelper;
typedef MCAS<T> t_MCAS;

  public:
    t_CasRow * cr;
    t_CasRow *last_row;

    MCASHelper<T>(t_CasRow *c, t_CasRow *l) {
      cr = c;
      last_row = l;
    };

    bool advance_watch(std::atomic<void *> *address, T value);

    void * complete(void * v, std::atomic<void *> *address);

    static T mcas_remove(T t, std::atomic<T> *address, t_CasRow *last_row);
};

}  // End mcas namespace
}  // End ucf nampespace
#endif  // UCF_MCAS_HELPER_
