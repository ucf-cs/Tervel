#include "ucf_threading.h"

namespace ucf {
namespace thread {

uint64_t nThreads;
std::atomic<uint64_t> threadCount;

__thread uint64_t rDepth;
__thread uint64_t threadID;
__thread bool rReturn;

namespace rc {

__thread PoolElem * tl_safe_pool = nullptr;
__thread PoolElem * tl_unsafe_pool = nullptr;
std::atomic<PoolElem *> gl_safe_pool;
std::atomic<PoolElem *> gl_unsafe_pool;

#ifdef DEBUG_POOL
__thread uint64_t tl_safe_pool_count = 0;
__thread uint64_t tl_unsafe_pool_count = 0;
std::atomic<uint64_t> gl_safe_pool_count;
std::atomic<uint64_t> gl_unsafe_pool_count;
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

bool PoolElem::watch(std::atomic<void *> *address, void *v, int pos) {
  hazardPointers[threadID*ID::END + pos].store(v);
  if (address->load() != v) {
    hazardPointers[threadID*ID::END + pos].store(nullptr);
    return false;
  } else {
    // return v->advanceWatch(address, v, pos);
    // Must call this from caller!
    return true;
  }
}

bool PoolElem::isWatched(PoolElem *p) {
  size_t i;
  for (i = 0; i < nThreads*ID::END; i++) {
    if (hazardPointers[i].load() == p) {
      return true;
    }
  }
  return p->advanceIsWatched();
}

void PoolElem::emptyThreadPool() {
  tryToFreeUnSafePool(false);

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

void PoolElem::emptyGlobalPool() {
  PoolElem *list = gl_unsafe_pool.exchange(nullptr);

  while (list) {
    PoolElem *temp = list->pool_next;
    list->unsafeFree();
    list = temp;
  }
}

void PoolElem::tryToFreeUnSafePool(bool force) {
  while (tl_unsafe_pool) {
    PoolElem *temp = tl_unsafe_pool->pool_next;

    bool watched = isWatched(temp);
    if (!force && watched) {
      break;
    } else {
      temp->unsafeFree();
      tl_unsafe_pool = temp;
    }
  }

  if (tl_unsafe_pool != nullptr) {
    PoolElem *prev = tl_unsafe_pool;
    PoolElem *temp = tl_unsafe_pool->pool_next;

    while (temp) {
      PoolElem *temp3 = temp->pool_next;
      bool watched = isWatched(temp);

      if (!force && watched) {
        prev = temp;
        temp = temp3;
      } else {
        temp->unsafeFree();
        prev->pool_next = temp3;
        temp = temp3;
      }
    }
  }
}

}  // namespace hp

__thread uint64_t helpID;
__thread uint64_t delayCount;

void ProgressAssurance::tryToHelp() {
  if (delayCount++ > helpDelay) {
    delayCount = 0;

    OpRecord *op = opTable[helpID].load();
    if (op != nullptr) {
      int pos =  hp::ID::id_oprec;
      std::atomic<void *> *temp =
          reinterpret_cast<std::atomic<void *> *>(&(opTable[helpID]));
      bool res = hp::PoolElem::watch(temp, op, pos);
      if (res) {
        if (op->advanceWatch(temp, op, pos)) {
          op->helpComplete();
          hp::PoolElem::unwatch(op, pos);
        }
      }
    }

    helpID = (helpID + 1) % nThreads;
  }
}

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
