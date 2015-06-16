#include <string>
#include <atomic>

// #include "myMemoryPool.h"
#include "Node.h"

//thread_local CMemPool *memoryPool;
#define MAX_CAPACITY 100000000
template<class T> class TestClass {
private:
      atomic<Node<long>> top_;
      long capacity;
      atomic<Node<T>> *storage;
public:      
     

  TestClass(size_t num_threads) {
      this->capacity = MAX_CAPACITY;
      storage = new atomic<Node<T>>[this->capacity];
      Node<long> topNode(-1);
      top_.store(topNode);
  }

  ~TestClass() {
      delete[] storage;
  }

  std::string toString() {
    return "Array Based Stack";
  };

  void attach_thread() {};

  void detach_thread() {};      
 
      bool push(T value) {
        for (;;){

          Node<long> topNode = top_.load();
          long oldTop = topNode.getValue();
          long oldTopTag = topNode.tag;
          if (oldTop >= capacity)
                  return false;
          Node<T> newNode(value);

          long index = oldTop+1;
          if (index >= capacity) {
              return false;
          }
          Node<T> oldNode = storage[index].load();
          newNode.tag.store(oldNode.tag + 1);

          Node<long> newTopNode(index);
          newTopNode.tag.store(oldTopTag+1);

          if(top_.compare_exchange_strong(topNode,newTopNode)){

              if (storage[index].compare_exchange_strong(oldNode, newNode)) {
                  return true;
              } else {
                  if(top_.compare_exchange_strong(newTopNode,topNode)){
                    return false;
                  }else{
                    continue;
                  }
              }       

          }else{
            continue;
          }
        }

      }
 
      bool peek(T &value) {
            if (top_.load().getValue() <= -1)
               return false;   
            value = storage[top_.load().getValue()].load().getValue();
            return true;
      }
 
      bool pop(T &value) {
          for (;;) {
              Node<long> topNode = top_.load();
              long index = topNode.getValue();
              long oldTopTag = topNode.tag;
              if (index <= -1)
                      return false;
              Node<T> oldNode = storage[index].load();
              Node<T> newNode(oldNode.getValue());
              newNode.tag.store(oldNode.tag + 1);  

              Node<long> newTopNode(index - 1);
              newTopNode.tag.store(oldTopTag + 1);                                             

              value = oldNode.getValue();
              if(top_.compare_exchange_strong(topNode,newTopNode)){   
                if (storage[index].compare_exchange_strong(oldNode, newNode)) {
                    return true;
                } else {
                    if(top_.compare_exchange_strong(newTopNode,topNode)){
                      return false;
                    }else{
                      continue;
                    }
                }
              }else{
                continue;
              }
          }
      }
};