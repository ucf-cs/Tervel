/*
 * File:   Node.h
 * Author: ramin
 *
 * Created on November 18, 2014, 1:53 PM
 */

#ifndef NODE_H  // ISSUE(steven): header guards should be full file path
#define	NODE_H
#include <atomic>
// #include "myMemoryPool.h"

// thread_local CMemPool *memoryPool;
using namespace std;  // ISSUE(steven): this is bad habit when working with libraries, you want to be explicit which functions you use

template<class T> class Node {  // ISSUE(steven): should be two lines, should use typename instead of class
public:  // ISSUE(steven): missing space
    Node(T) noexcept ;
    Node() noexcept ;
    Node(const Node& orig) noexcept;
    // virtual ~Node() noexcept {

    // }
    T getValue(){
        return value;
    }
    void setValue(T v){
        value = v;
    }
    // void* operator new(size_t sz);
    // void operator delete(void* ptr) noexcept;
    atomic<unsigned long> tag;      // ISSUE(steven): trailing whitespace. why is this atomic? why is this not a constant?


private:
    T value;

};

template <class T>Node<T>::Node() noexcept {
    tag = 0;
}

template <class T>Node<T>::Node(T v) noexcept{
    value = v;
    tag = 0;
}


/*
template <class T> void* Node<T>::operator new(size_t sz) {
//    printf("global op new called, size = %zu\n",sz);
    if(memoryPool == NULL){
//        cout << " memory pool was null for me ! " << this_thread::get_id() << endl;
        memoryPool = new CMemPool(10,sizeof(Node<T>));
    }else{
//        cout << " memory pool was not null for me ! " << this_thread::get_id() << endl;
    }
    Node<T>* mem = (Node<T> *)memoryPool->Alloc(1);
//    cout << "allocate " << mem << " to " << this_thread::get_id() << endl;
    return mem;
}

template <class T> void Node<T>::operator delete(void* ptr) noexcept
{
//    std::puts("global op delete called");
    if(memoryPool == NULL){
//        cout << " memory pool was null for me in remove ! " << this_thread::get_id() << endl;
//        memoryPool = new CMemPool(10,sizeof(Node<T>));
        return;
    }else{
//        cout << " memory pool was not null for me in remove ! " << this_thread::get_id() << endl;
    }
//    std::free(ptr);
//    cout << "free" << this_thread::get_id() << endl;
//    cout << "free " << ptr << " from " << this_thread::get_id() << endl;
    memoryPool->Free(ptr);
}*/

template <class T>Node<T>::Node(const Node& orig) noexcept {
    tag.store(orig.tag.load());
    value = orig.value;
}





#endif	/* NODE_H */

