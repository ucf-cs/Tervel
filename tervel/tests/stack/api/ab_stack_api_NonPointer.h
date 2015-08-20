#include <string>
#include <atomic>

// #include "myMemoryPool.h"
#include "Node.h"
  // ISSUE(steven): Node class should be an inner class of TestClass (which should be named Stack something...)

//thread_local CMemPool *memoryPool;
#define MAX_CAPACITY 100000000
template<class T> class TestClass {
private:
      atomic<Node<long>> top_;
      long capacity;  // ISSUE(steven): member variables should end with _, should be const
      atomic<Node<T>> *storage;  // ISSUE(steven): member variables should end with
public:


  TestClass(size_t num_threads) {
      this->capacity = MAX_CAPACITY;   // ISSUE(steven): capacity should be const and should be done assigned using initializer list, MAX_CAPACITY should be a default value for a constructor argument
      storage = new atomic<Node<T>>[this->capacity];
      Node<long> topNode(-1);  // ISSUE(steven): long should be T?
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

      bool push(T value) {  // ISSUE(steven): bad indention
        for (;;){  // ISSUE(steven): missing space before {

          Node<long> topNode = top_.load();  // ISSUE(steven): long should be T?
          long oldTop = topNode.getValue();
          long oldTopTag = topNode.tag;  // ISSUE(steven): this is an atomic load, should access through a member function or at least have .load() at the end
          if (oldTop >= capacity)  // ISSUE(steven): inconsistent lack of {}
                  return false;  // ISSUE(steven): inconsistent indention
          Node<T> newNode(value);

          long index = oldTop+1;
          if (index >= capacity) {
              return false;  // ISSUE(steven): inconsistent indention
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

      bool peek(T &value) {  // ISSUE(steven): inconsistent indentation
            if (top_.load().getValue() <= -1)
               return false;
            // ISSUE(steven): wh
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