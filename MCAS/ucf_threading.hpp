//
//  main_tester.c
//  mCAS
//
//  Created by Steven Feldman on 3/25/12.
//  Copyright (c) 2012 Steven FELDMAN. All rights reserved.
//

#ifndef UCF_THREADING_HPP_
#define UCF_THREADING_HPP_

#include <atomic>
#include <cstdint>

#include <stdlib.h>
#include <assert.h>
#define DEBUG_POOL 1

typedef std::uint64_t int64;

namespace ucf {
namespace thread {
  constexpr size_t helpDelay = 1;
  constexpr size_t ALIGNLEN = 64;

  extern int64 nThreads;
  extern std::atomic<int64> threadCount;

  extern __thread int64 rDepth;
  extern __thread int64 threadID;
  extern __thread bool rReturn;

  namespace rc {
    class PoolElem;

    constexpr bool NO_REUSE_MEM = false;

    extern __thread PoolElem * tl_safe_pool;
    extern __thread PoolElem * tl_unsafe_pool;
    extern std::atomic<PoolElem *> gl_safe_pool;
    extern std::atomic<PoolElem *> gl_unsafe_pool;

#ifdef DEBUG_POOL
    extern __thread int64 tl_safe_pool_count;
    extern __thread int64 tl_unsafe_pool_count;
    extern std::atomic<int64> gl_safe_pool_count;
    extern std::atomic<int64> gl_unsafe_pool_count;
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
      virtual void helpComplete() { assert(false); }

      virtual void * getLogicalValue(void * t, std::atomic<void *> *address) {
        assert(false);
        return nullptr;
      };

      bool advanceWatch(std::atomic<void *> *address, void * p) { return true; }
      void advanceunwatch() {}
      bool advanceIsWatched() { return false; }

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

      static bool watch(Descriptor* obj, std::atomic<void *> *a, void * value);

      static void unwatch(Descriptor* obj);

      static bool isWatched(Descriptor *obj);

      static void addToSafe(Descriptor *descr);

      static void addToUnSafe(Descriptor *descr);

      static void tryToFreeUnSafePool(bool force=false);

      // Sends it to the global pool
      static void emptySafePool();

      // Sends it to the safe pool
      static void emptyUnSafePool();

      static void emptyThreadPools() {
        emptyUnSafePool();
        emptySafePool();
      };

      static void emptyGlobalPools();

      static PoolElem * getFromPool();

      static void * getDescriptor(size_t s);

      // void * operator new(size_t) { assert(false); return nullptr; }
    };  // End Pool Elem class

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

  }  // namespace rc


  namespace hp {
    class PoolElem;

    constexpr bool NO_FREE_MEMORY = false;
    enum ID {
      id_mcas,
      id_oprec,
      END
    };

    extern __thread PoolElem * tl_unsafe_pool;
    extern std::atomic<PoolElem *> gl_unsafe_pool;
    extern std::atomic<void *> *hazardPointers;

    void Initilize_Hazard_Pointers();

    void Destroy_Hazard_Pointers();

    class PoolElem {
    public:
      PoolElem *pool_next;

      void init() {
        pool_next = nullptr;
      };

      PoolElem() {}
      virtual ~PoolElem() {}
      virtual bool advanceWatch(std::atomic<void *> *address, void *v,
          int pos) { return true; }
      virtual void advanceunwatch(int pos) {}
      virtual bool advanceIsWatched() { return false; }
      virtual void safeFree() { assert(false); }
      virtual void unsafeFree() { assert(false); }

      static bool watch(std::atomic<void *> *address, void *v, int pos) {
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
      }  // End tryToFreeUnSafePool
    };
  }  // namespace hp

  extern __thread int64 helpID;
  extern __thread int64 delayCount;

  class OpRecord: public hp::PoolElem {
   public:
    static constexpr size_t MAX_FAILURE = 1;
    OpRecord() {}
    virtual ~OpRecord() {}

    virtual void helpComplete() {
      assert(false);
    }
  };

  class ProgressAssurance {
   public:
    std::atomic<OpRecord *> *opTable;
    int nThreads;
    explicit ProgressAssurance(int _nThreads) {
      nThreads = _nThreads;
      opTable = new std::atomic<OpRecord *>[nThreads]();
    }

    ~ProgressAssurance() {
      delete opTable;
    }

    void tryToHelp() {
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

    void askForHelp(OpRecord *op) {
      opTable[threadID].store(op);
      op->helpComplete();
      opTable[threadID].store(nullptr);
      return;
    }
  };  // End Progress Assurance Class

  extern ProgressAssurance *progressAssurance;

  void Initilize_Threading_Manager(int maxUniqueThreads);

  void Destory_Threading_Manager();

  void attachThread();

  void dettachThread();

}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREADING_HPP_
