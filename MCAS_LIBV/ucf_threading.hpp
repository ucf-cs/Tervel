//
//  main_tester.c
//  mCAS
//
//  Created by Steven Feldman on 3/25/12.
//  Copyright (c) 2012 Steven FELDMAN. All rights reserved.
//

#include <atomic>
#include <cstdint>

#include <stdlib.h>
#include <assert.h>
#define DEBUG_POOL 1

typedef std::uint64_t int64;

namespace ucf {
namespace thread {
  const size_t helpDelay = 1;
  int64 nThreads;
  std::atomic<int64> threadCount;

  __thread int64 rDepth, threadID, helpID, delayCount;
  __thread bool rReturn;

  const size_t ALIGNLEN = 64;

  namespace rc {
    const bool NO_REUSE_MEM = false;

    class PoolElem;
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

    class Descriptor {
    public:
      Descriptor() {}
      virtual ~Descriptor() {}

      virtual void unsafeFree();
      virtual void safeFree();


      virtual void * descr_complete(void * t, std::atomic<void *> *address) {
        assert(false);
        return nullptr;
      };
      virtual void helpComplete() {
        assert(false);
      }

      virtual void * getLogicalValue(void * t, std::atomic<void *> *address) {
        assert(false);
        return nullptr;
      };

      bool advanceWatch(std::atomic<void *> *address, void * p) {
        return true;
      }
      void advanceunwatch() {}
      bool advanceIsWatched() {
        return false;
      }

      template<class T>
      static T mark(Descriptor *descr) {
        int64 temp = reinterpret_cast<int64>(descr);
        return reinterpret_cast<T>(temp | 0x1L);
      }

      template<class T>
      static Descriptor * unmark(T descr) {
        int64 temp = reinterpret_cast<int64>(descr);
        return reinterpret_cast<Descriptor *> (temp & ~0x1L);
      }

      template<class T>
      static bool isDescr(T descr) {
        int64 temp = reinterpret_cast<int64>(descr);
        return (0x1L == (temp & 0x1L));
      }

      template<class T>
      static T remove(T t, std::atomic<T> *address);

      void * operator new(size_t s);
    };  // End class Descriptor

    class PoolElem {
    public:
      PoolElem *pool_next;
      std::atomic<int64> rc_count;
#ifdef DEBUG_POOL
      int64 type = 0;
      std::atomic<int64> allocation_count;
      std::atomic<int64> free_count;
#endif


      void init() {
        pool_next = nullptr;
        rc_count.store(0);

#ifdef DEBUG_POOL
        type = 69;
        allocation_count.store(1);
        free_count.store(0);
#endif
      };

      static PoolElem * getPoolElemRef(Descriptor *p) {
        PoolElem *temp = reinterpret_cast<PoolElem *>(p);
        return (temp - 1);
      }
      static Descriptor * getDescrRef(PoolElem *p) {
        return reinterpret_cast<Descriptor *>(p + 1);
      }

      static bool watch(Descriptor* obj, std::atomic<void *> *a, void * value) {
        PoolElem * rcObj = getPoolElemRef(obj);
        rcObj->rc_count.fetch_add(1);
        if (a->load() != value) {
          rcObj->rc_count.fetch_add(-1);
          return false;
        } else {
          return obj->advanceWatch(a, value);
        }
      }

      static void abort_watch(Descriptor* obj) {
        PoolElem * rcObj = getPoolElemRef(obj);
        rcObj->rc_count.fetch_add(-1);
      }
      static void unwatch(Descriptor* obj) {
        PoolElem * rcObj = getPoolElemRef(obj);
        rcObj->rc_count.fetch_add(-1);
        obj->advanceunwatch();
      }

      static bool isWatched(Descriptor *obj) {
        PoolElem * rcObj = getPoolElemRef(obj);
        if (rcObj->rc_count.load() == 0) {
          return obj->advanceIsWatched();
        } else {
          return true;
        }
      }

      static void addToSafe(Descriptor *descr) {
        PoolElem *p = getPoolElemRef(descr);

#ifdef DEBUG_POOL
        p->free_count.fetch_add(1);
        assert(p->free_count.load() == p->allocation_count.load());
        tl_safe_pool_count++;
#endif
        p->pool_next = tl_safe_pool;
        tl_safe_pool = p;
      };

      static void addToUnSafe(Descriptor *descr) {
        PoolElem *p = getPoolElemRef(descr);
        p->pool_next = tl_unsafe_pool;
        tl_unsafe_pool = p;

#ifdef DEBUG_POOL
          tl_unsafe_pool_count++;
#endif
      };



      static void tryToFreeUnSafePool(bool force = false) {
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
            }  // End Else
          }  // End While
        }  // End If tl_pool
      };  // End tryToFreeUnSafePool

      static void emptySafePool() {  // Sends it to Global pool
        if (tl_safe_pool != nullptr) {
          PoolElem *p1 = tl_safe_pool;
          PoolElem *p2 = p1->pool_next;
          while (p2 != nullptr) {
            p1 = p2;
            p2 = p1->pool_next;
          }

          p1->pool_next = gl_safe_pool.load();
          while (
                  !gl_safe_pool.compare_exchange_strong
                      (p1->pool_next, tl_unsafe_pool)
                ) {}
            // p1->pool_next's value is updated to the current value
            // after a failed cas. (pass by reference fun)
        }
        tl_safe_pool = nullptr;
      };

      static void emptyUnSafePool() {  // Sends it  safePool
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
      };

      static void emptyThreadPools() {
        emptyUnSafePool();
        emptySafePool();
      };

      static void emptyGlobalPools() {
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


      static PoolElem * getFromPool() {
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
      };

      static void * getDescriptor(size_t s) {
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

      void * operator new(size_t size) {
        assert(false);
      };  // End  new operator
    };  // End Pool Elem class

    void * Descriptor::operator new(size_t s) {
        return PoolElem::getDescriptor(s);
    }


    template<class T>
    T Descriptor::remove(T t, std::atomic<T> *address) {
      if (rDepth > thread::nThreads+1) {
        rReturn = true;
        return reinterpret_cast<T>(nullptr);  // result not used
      }
      rDepth++;

      Descriptor *descr = Descriptor::unmark(t);

      bool watched = PoolElem::watch(descr,
                    reinterpret_cast<std::atomic<void *>*>(address),
                    reinterpret_cast<void *>(t));
      T newValue;
      if (watched) {
        newValue = reinterpret_cast<T>(descr->descr_complete(
            reinterpret_cast<void *>(t),
            reinterpret_cast<std::atomic<void *>*>(address)));

        PoolElem::unwatch(descr);
      } else {
        newValue = address->load();
      }

      rDepth--;
      return newValue;
    }
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
  }  // end rc pool name space


  namespace hp {
    const bool NO_FREE_MEMORY = false;

    class PoolElem;
    __thread PoolElem * tl_unsafe_pool = nullptr;
    std::atomic<PoolElem *> gl_unsafe_pool;

    enum ID {id_mcas, id_oprec, END};
    std::atomic<void *> *hazardPointers;
    static void Initilize_Hazard_Pointers() {
      hazardPointers = new std::atomic<void *>[nThreads*ID::END]();
      gl_unsafe_pool.store(nullptr);
    }
    static void Destroy_Hazard_Pointers() {
      delete[] hazardPointers;
      assert(tl_unsafe_pool == nullptr);
    }

    class PoolElem {
    public:
      PoolElem *pool_next;

      void init() {
        pool_next = nullptr;
      };

      PoolElem() {}
      virtual ~PoolElem() {}
      virtual bool advanceWatch(std::atomic<void *> *address,
                                          void *v, int pos) {
        return true;
      }
      virtual void advanceunwatch(int pos) {}
      virtual bool advanceIsWatched() {
        return false;
      }
      virtual void safeFree() {
        assert(false);
      }
      virtual void unsafeFree() {
        assert(false);
      }

      static bool watch(std::atomic<void *> *address,
                                  void *v, int pos) {
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

      static void watch(void *v, int pos) {
        hazardPointers[threadID*ID::END + pos].store(v);
      }

      static void abort_watch(int pos) {
        hazardPointers[threadID*ID::END + pos].store(nullptr);
      }

      static void unwatch(PoolElem *p, int pos) {
        hazardPointers[threadID*ID::END + pos].store(nullptr);
        p->advanceunwatch(pos);
      }
      static void unwatch(int pos) {
        hazardPointers[threadID*ID::END + pos].store(nullptr);
      }

      static bool isWatched(PoolElem *p) {
        size_t i;
        for (i = 0; i < nThreads*ID::END; i++) {
          if (hazardPointers[i].load() == p) {
            return true;
          }
        }
        return p->advanceIsWatched();
      }


      static void addToUnSafe(PoolElem *p) {
        p->pool_next = tl_unsafe_pool;
        tl_unsafe_pool = p;
      };

      static void emptyThreadPool() {
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
      };

      static void emptyGlobalPool() {
        PoolElem *list = gl_unsafe_pool.exchange(nullptr);

        while (list) {
          PoolElem *temp = list->pool_next;
          list->unsafeFree();
          list = temp;
        }
      }

      static void tryToFreeUnSafePool(bool force = false) {
        while (tl_unsafe_pool) {
          PoolElem *temp = tl_unsafe_pool->pool_next;

          bool watched = isWatched(temp);
          if (!force && watched) {
            break;
          } else {
            temp->unsafeFree();
            tl_unsafe_pool = temp;
          }
        }  // End While Unsafe pool

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
            }  // End Else
          }  // End While
        }  // End If tl_pool
      };  // End tryToFreeUnSafePool
    };
  };  // End hp namespace

  class OpRecord: public hp::PoolElem {
  public:
    static const size_t MAX_FAILURE = 1;
    OpRecord() {}
    virtual ~OpRecord() {}

    virtual void helpComplete() {
      assert(false);
    }
  };

  std::atomic<OpRecord *> *opTable;

  static void Initilize_Threading_Manager(int maxUniqueThreads) {
    nThreads = maxUniqueThreads;
    opTable = new std::atomic<OpRecord *>[nThreads]();
    threadCount.store(0);

    hp::Initilize_Hazard_Pointers();
  }
  static void Destory_Threading_Manager() {
    delete[] opTable;
    hp::Destroy_Hazard_Pointers();
    rc::PoolElem::emptyGlobalPools();
    hp::PoolElem::emptyGlobalPool();
  }
  static void attachThread() {
    threadID = threadCount.fetch_add(1);
    helpID = 0;
    rDepth = 0;
    delayCount = 0;
    rReturn = false;
  }
  static void dettachThread() {
    rc::PoolElem::emptyThreadPools();
    hp::PoolElem::emptyThreadPool();
  }

  static void tryToHelp() {
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

  static void askForHelp(OpRecord *op) {
    opTable[threadID].store(op);
    op->helpComplete();
    opTable[threadID].store(nullptr);
    return;
  }

}  // End Thread namespace
}  // End UCF namespace
