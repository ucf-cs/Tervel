#include "ucf_threading.hpp"

namespace ucf {
namespace thread {

int64 nThreads;
std::atomic<int64> threadCount;

__thread int64 rDepth;
__thread int64 threadID;
__thread bool rReturn;

namespace rc {

__thread PoolElem * tl_safe_pool = nullptr;
__thread PoolElem * tl_unsafe_pool = nullptr;
std::atomic<PoolElem *> gl_safe_pool;
std::atomic<PoolElem *> gl_unsafe_pool;

#ifdef DEBUG_POOL
__thread int64 tl_safe_pool_count = 0;
__thread int64 tl_unsafe_pool_count = 0;
std::atomic<int64> gl_safe_pool_count;
std::atomic<int64> gl_unsafe_pool_count;
#endif

void Descriptor::unsafeFree() {
  PoolElem::addToSafe(this);
}

void Descriptor::safeFree() {
  if (PoolElem::isWatched(this)) {
    PoolElem::addToUnSafe(this);
  } else {
    unsafeFree();
  }
}

void * Descriptor::operator new(size_t s) {
  return PoolElem::getDescriptor(s);
}

bool PoolElem::watch(Descriptor* obj, std::atomic<void *> *a, void * value) {
  PoolElem * rcObj = getPoolElemRef(obj);
  rcObj->rc_count.fetch_add(1);
  if (a->load() != value) {
    rcObj->rc_count.fetch_add(-1);
    return false;
  } else {
    bool res = obj->advanceWatch(a, value);
    if (res) {
      return true;
    } else {
      rcObj->rc_count.fetch_add(-1);
      return false;
    }
  }
}

void PoolElem::unwatch(Descriptor* obj) {
  PoolElem * rcObj = getPoolElemRef(obj);
  rcObj->rc_count.fetch_add(-1);
  obj->advanceunwatch();
}

bool PoolElem::isWatched(Descriptor *obj) {
  PoolElem * rcObj = getPoolElemRef(obj);
  if (rcObj->rc_count.load() == 0) {
    return obj->advanceIsWatched();
  } else {
    return true;
  }
}

void PoolElem::addToSafe(Descriptor *descr) {
  PoolElem *p = getPoolElemRef(descr);

#ifdef DEBUG_POOL
  p->free_count.fetch_add(1);
  assert(p->free_count.load() == p->allocation_count.load());
  tl_safe_pool_count++;
#endif
  p->pool_next = tl_safe_pool;
  tl_safe_pool = p;
}

void PoolElem::addToUnSafe(Descriptor *descr) {
  PoolElem *p = getPoolElemRef(descr);
  p->pool_next = tl_unsafe_pool;
  tl_unsafe_pool = p;

#ifdef DEBUG_POOL
    tl_unsafe_pool_count++;
#endif
}

void PoolElem::tryToFreeUnSafePool(bool force) {
  while (tl_unsafe_pool) {
    PoolElem *temp = tl_unsafe_pool->pool_next;
    Descriptor *temp_descr = getDescrRef(tl_unsafe_pool);

    bool watched = isWatched(temp_descr);
    if (!force && watched) {
      break;
    } else {
#ifdef DEBUG_POOL
      tl_unsafe_pool_count--;
#endif
      temp_descr->unsafeFree();
      tl_unsafe_pool = temp;
    }
  }  // End While Unsafe pool

  if (tl_unsafe_pool != nullptr) {
    PoolElem *prev = tl_unsafe_pool;
    PoolElem *temp = tl_unsafe_pool->pool_next;

    while (temp) {
      Descriptor *temp_descr = getDescrRef(temp);
      PoolElem *temp3 = temp->pool_next;

      bool watched = isWatched(temp_descr);
      if (!force && watched) {
        prev = temp;
        temp = temp3;
      } else {
        temp_descr->unsafeFree();
        prev->pool_next = temp3;
        temp = temp3;
#ifdef DEBUG_POOL
        tl_unsafe_pool_count--;
#endif
      }   // End Else
    }   // End While
  }   // End If tl_pool
}   // End tryToFreeUnSafePool

void PoolElem::emptySafePool() {
  if (tl_safe_pool != nullptr) {
    PoolElem *p1 = tl_safe_pool;
    PoolElem *p2 = p1->pool_next;
    while (p2 != nullptr) {
      p1 = p2;
      p2 = p1->pool_next;
    }

    p1->pool_next = gl_safe_pool.load();
    while (
        !gl_safe_pool.compare_exchange_strong(p1->pool_next, tl_unsafe_pool)) {}
      // p1->pool_next's value is updated to the current value
      // after a failed cas. (pass by reference fun)
  }
  tl_safe_pool = nullptr;
}

void PoolElem::emptyUnSafePool() {
  tryToFreeUnSafePool(true);
  if (tl_unsafe_pool != nullptr) {
    PoolElem *p1 = tl_unsafe_pool;
    PoolElem *p2 = p1->pool_next;
    while (p2 != nullptr) {
      p1 = p2;
      p2 = p1->pool_next;
    }

    p1->pool_next = gl_unsafe_pool.load();
    while (
            !gl_unsafe_pool.compare_exchange_strong
                (p1->pool_next, tl_unsafe_pool)
          ) {}
      // p1->pool_next's value is updated to the current value
      // after a failed cas. (pass by reference fun)
  }
  tl_unsafe_pool = nullptr;
}

void PoolElem::emptyGlobalPools() {
  PoolElem *list = gl_unsafe_pool.exchange(nullptr);

  while (list) {
    PoolElem *temp = list->pool_next;
    Descriptor *temp_descr = getDescrRef(list);
    temp_descr->unsafeFree();
    list = temp;
  }

  list = gl_safe_pool.exchange(nullptr);

  while (list) {
    PoolElem *temp = list->pool_next;
    delete list;
    list = temp;
  }
}

PoolElem * PoolElem::getFromPool() {
  /**
      First Try to move any object from the unsafe list to the safe list
  **/
  tryToFreeUnSafePool();  // false

  if (!NO_REUSE_MEM && tl_safe_pool) {
    PoolElem *temp = tl_safe_pool;
    tl_safe_pool = tl_safe_pool->pool_next;

    #ifdef DEBUG_POOL
      assert(temp->free_count.load() == temp->allocation_count.load());
      temp->allocation_count.fetch_add(1);
      tl_safe_pool_count--;
    #endif

    temp->pool_next = nullptr;
    return temp;
  } else {
    return nullptr;
  }
}

void * PoolElem::getDescriptor(size_t s) {
  assert(s <= ALIGNLEN);
  PoolElem * temp = getFromPool();
  if (temp == nullptr) {
    temp = reinterpret_cast<PoolElem *>(new char[ALIGNLEN]);
    if (temp == nullptr) {
      exit(-1);
    } else {
      temp->init();
     // printf("New: (%ld) %p, %p\n", threadID, temp, getDescrRef(temp));
    }
  } else {
  // printf("Reusing: (%ld) %p, %p\n", threadID, temp, getDescrRef(temp));
  }
  return getDescrRef(temp);
}

}  // namespace rc

namespace hp {

__thread PoolElem * tl_unsafe_pool = nullptr;
std::atomic<PoolElem *> gl_unsafe_pool;
std::atomic<void *> *hazardPointers;

void Initilize_Hazard_Pointers() {
  hazardPointers = new std::atomic<void *>[nThreads*ID::END]();
  gl_unsafe_pool.store(nullptr);
}

void Destroy_Hazard_Pointers() {
  delete[] hazardPointers;
  assert(tl_unsafe_pool == nullptr);
}

}  // namespace hp

__thread int64 helpID;
__thread int64 delayCount;

ProgressAssurance *progressAssurance;

void Initilize_Threading_Manager(int maxUniqueThreads) {
  nThreads = maxUniqueThreads;
  progressAssurance =  new ProgressAssurance(nThreads);
  threadCount.store(0);

  hp::Initilize_Hazard_Pointers();
}

void Destory_Threading_Manager() {
  delete progressAssurance;
  hp::Destroy_Hazard_Pointers();
  rc::PoolElem::emptyGlobalPools();
  hp::PoolElem::emptyGlobalPool();
}

void attachThread() {
  threadID = threadCount.fetch_add(1);
  helpID = 0;
  rDepth = 0;
  delayCount = 0;
  rReturn = false;
}

void dettachThread() {
  rc::PoolElem::emptyThreadPools();
  hp::PoolElem::emptyThreadPool();
}

}  // namespace thread
}  // namespace ucf
