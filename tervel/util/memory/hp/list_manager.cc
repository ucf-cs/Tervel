#include <tervel/util/memory/hp/list_manager.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

namespace tervel {
namespace util {
namespace memory {
namespace hp {

ListManager:: ~ListManager() {
  for (size_t i = 0; i < number_pools_; i++) {
    Element * element = free_lists_[i].element_list_.load();
    while (element != nullptr) {
      Element *cur = element;
      element = element->next();

      bool watched = tervel::util::memory::hp::HazardPointer::is_watched(cur);
      assert(!watched && "A Hazard Pointer Protected is still a watched when the list manager is being freed");
      #ifndef TERVEL_MEM_HP_NO_FREE
        delete cur;
      #endif
    }  // While elements to be freed
  }  // for pool
  // delete free_lists_; // std::unique_ptr causes this array to be destroyed
};

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel