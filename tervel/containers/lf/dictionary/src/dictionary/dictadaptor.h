#ifndef DICTADAPTOR_H
#define DICTADAPTOR_H

#include <tbb/concurrent_unordered_map.h>
#include <map>
#include "dictionary/mdlist/mdlist.h"
#include "dictionary/herlihy/SkipList.hpp"
#include "dictionary/ellen/NBBST.hpp"
#include "dictionary/bronson/AVLTree.hpp"

extern "C"
{
#include "dictionary/fraser/set.h"
#include "dictionary/dick/intset.h"
#include "dictionary/herlihyoptimistic/optimistic.h"
#include "dictionary/citrus/citrus.h"
}

extern __thread unsigned int thread_num;

template<typename T>
class DictAdaptor
{
};

template<>
class DictAdaptor<MDList>
{
public:
    DictAdaptor(uint32_t testSize, uint32_t numThread, uint32_t keyRange)
        :m_dict(testSize, numThread, keyRange)
    {}

    void Init(uint32_t threadId)
    {
        m_dict.m_pool.Init(threadId);
    }

    void Uninit()
    {
        m_dict.m_pool.Uninit();
    }

    void Insert(uint32_t key)
    {
        m_dict.Insert(key);
    }

    void Delete(uint32_t key)
    {
        m_dict.Delete(key);
    }

    void Find(uint32_t key)
    {
        m_dict.Find(key);
    }

private:
    MDList m_dict;
};


//-----------------------------------------------------------------------------------
typedef tbb::concurrent_unordered_map<uint32_t, uint32_t > TBBMAP;

template<>
class DictAdaptor<TBBMAP>
{
public:
    void Init(uint32_t threadId){}
    void Uninit(){}

    void Insert(uint32_t key)
    {
        m_map.insert(std::make_pair(key, key));
    }

    void Delete(uint32_t key)
    {
        m_map.unsafe_erase(key);
    }

    void Find(uint32_t key)
    {
        m_map.find(key);
    }

private:
    TBBMAP m_map;
};


//-----------------------------------------------------------------------------------
typedef std::map<uint32_t, uint32_t> STDMAP;

template<>
class DictAdaptor<STDMAP>
{
public:
    void Init(uint32_t threadId){}
    void Uninit(){}

    void Insert(uint32_t key)
    {
        m_mutex.lock();
        m_map[key] = key;
        m_mutex.unlock();
    }

    void Delete(uint32_t key)
    {
        m_mutex.lock();
        m_map.erase(key);
        m_mutex.unlock();
    }

    void Find(uint32_t key)
    {
        m_mutex.lock();
        m_map.find(key);
        m_mutex.unlock();
    }

private:
    STDMAP m_map;
    std::mutex m_mutex;
};


//-----------------------------------------------------------------------------------
typedef fr_set_t FRDICT;

template<>
class DictAdaptor<FRDICT>
{
public:
    void Init(uint32_t threadId){}
    void Uninit(){}

    DictAdaptor()
    {
        fr_init_set_subsystem();
        m_dict = set_alloc();
    }

    ~DictAdaptor()
    {
        fr_destroy_set_subsystem();
    }

    void Insert(uint32_t key)
    {
        set_update(m_dict, key, &key, 0);
    }

    void Delete(uint32_t key)
    {
        set_remove(m_dict, key);
    }

    void Find(uint32_t key)
    {
        set_lookup(m_dict, key);
    }

private:
    FRDICT* m_dict;
};

typedef Herlihy::SkipList<int, 129> HLDICT;

template<>
class DictAdaptor<HLDICT>
{
public:
    void Init(uint32_t threadId)
    {
        thread_num = threadId;
    }
    void Uninit(){}

    void Insert(uint32_t key)
    {
        m_dict.add(key);
    }

    void Delete(uint32_t key)
    {
        m_dict.remove(key);
    }

    void Find(uint32_t key)
    {
        m_dict.contains(key);
    }

private:
    HLDICT m_dict;
};

typedef Ellen::NBBST<int, 129> ELDICT;

template<>
class DictAdaptor<ELDICT>
{
public:
    void Init(uint32_t threadId)
    {
        thread_num = threadId;
    }

    void Uninit(){}

    void Insert(uint32_t key)
    {
        m_dict.add(key);
    }

    void Delete(uint32_t key)
    {
        m_dict.remove(key);
    }

    void Find(uint32_t key)
    {
        m_dict.contains(key);
    }

private:
    ELDICT m_dict;
};

typedef Broson::AVLTree<int, 129> BRDICT;

template<>
class DictAdaptor<BRDICT>
{
public:
    void Init(uint32_t threadId)
    {
        thread_num = threadId;
    }

    void Uninit(){}

    void Insert(uint32_t key)
    {
        m_dict.add(key);
    }

    void Delete(uint32_t key)
    {
        m_dict.remove(key);
    }

    void Find(uint32_t key)
    {
        m_dict.contains(key);
    }

private:
    BRDICT m_dict;
};

//-----------------------------------------------------------------------------------
typedef set_t DKDICT;

template<>
class DictAdaptor<DKDICT>
{
public:
    void Init(uint32_t threadId){}
    void Uninit(){}

    DictAdaptor()
    {
        ptst_subsystem_init();
        gc_subsystem_init();
        set_subsystem_init();
        m_dict = set_new(1); //autostart background thread
    }

    ~DictAdaptor()
    {
        gc_subsystem_destroy();
        set_delete(m_dict);
    }

    void Insert(uint32_t key)
    {
        sl_add_old(m_dict, key, 0);
    }

    void Delete(uint32_t key)
    {
        sl_remove_old(m_dict, key, 0);
    }

    void Find(uint32_t key)
    {
        sl_contains_old(m_dict, key, 0);
    }

private:
    DKDICT* m_dict;
};

//-----------------------------------------------------------------------------------
typedef hsl_intset HLODICT;

template<>
class DictAdaptor<HLODICT>
{
public:
    void Init(uint32_t threadId){}
    void Uninit(){}

    DictAdaptor()
    {
        m_dict = sl_set_new();
    }

    ~DictAdaptor()
    {
        sl_set_delete(m_dict);
    }

    void Insert(uint32_t key)
    {
        optimistic_insert(m_dict, key);
    }

    void Delete(uint32_t key)
    {
        optimistic_delete(m_dict, key);
    }

    void Find(uint32_t key)
    {
        optimistic_find(m_dict, key);
    }

private:
    HLODICT* m_dict;
};

//-----------------------------------------------------------------------------------
typedef ctr_node CTRDICT;

template<>
class DictAdaptor<CTRDICT>
{
public:
    void Init(uint32_t threadId)
    {
        urcu_register(threadId);
    }
    void Uninit(){}

    DictAdaptor(uint32_t numThread)
    {
        initURCU(numThread); // initialize RCU with specific numthreads
        m_dict = ctr_init();
    }

    ~DictAdaptor()
    {
    }

    void Insert(uint32_t key)
    {
        ctr_insert(m_dict, key, key);
    }

    void Delete(uint32_t key)
    {
        ctr_delete(m_dict, key);
    }

    void Find(uint32_t key)
    {
        ctr_contains(m_dict, key);
    }

private:
    CTRDICT m_dict;
};


#endif /* end of include guard: DICTADAPTOR_H */
