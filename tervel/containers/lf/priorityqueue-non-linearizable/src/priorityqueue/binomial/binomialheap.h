/**********************************************************
 * File: BinomialHeap.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * An implementation of a priority queue class backed by a
 * binomial heap.  A descripton of how such heaps work can
 * be found in "Introduction to Algorithms, Second Edition"
 * by Cormen, Leisserson, Rivest, and Stein.  The 
 * implementation contained in this file is optimized for
 * readability rather than speed, but all of the heap
 * operations have the correct asymptotic runtime.
 */

#ifndef BinomialHeap_Included
#define BinomialHeap_Included

#include <vector>    // For std::vector
#include <algorithm> // For std::for_each, std::min_element, std::swap, std::max
#include <stdlib.h>  // For size_t, NULL

/* Forward-declare utility node type.  The detail namespace holds 
 * implementation-specific helper functions and is not meant to be
 * used by clients.
 */
namespace detail {
  template <typename T> struct BinomialNode;
}

/* A class representing a binomial heap, which is a priority
 * queue supporting the following operations with the following
 * runtimes.  Because of character limitations in C++ code,
 * the @ sign should be taken to mean big-Theta
 *
 * Operation             | Runtime
 * ----------------------+----------
 * Create empty heap     | @(1)
 * Insert single element | O(lg N)
 * Query min value       | @(lg N)
 * Merge heaps           | O(lg N)
 * Delete min            | @(lg N)
 */
template <typename T> class BinomialHeap {
public:
  /* Constructor: BinomialHeap()
   * Usage: BinomialHeap<int> myHeap;
   * -------------------------------------------------------
   * Constructs a new, empty binomial heap.
   */
  BinomialHeap();

  /* Destructor: ~BinomialHeap()
   * Usage: (implicit)
   * --------------------------------------------------------
   * Deallocates the memory used by the binomial heap.
   */
  ~BinomialHeap();

  /* Copy functions: BinomialHeap(const BinomialHeap&);
   *         BinomialHeap& operator= (const BinomialHeap&)
   * --------------------------------------------------------
   * Constructs a copy of an existing binomial heap or sets
   * an existing binomial heap to be a copy of an existing
   * binomial heap.
   */
  BinomialHeap(const BinomialHeap&);
  BinomialHeap& operator= (const BinomialHeap&);
  
  /* void push(const T&);
   * Usage: myHeap.push(137);
   * --------------------------------------------------------
   * Adds a new element to the min-heap.
   */
  void push(const T&);

  /* const T& top() const;
   * Usage: cout << myHeap.top() << endl;
   * --------------------------------------------------------
   * Returns an immutable reference to the minimum element in
   * the heap.
   */
  const T& top() const;

  /* void pop();
   * Usage: myHeap.pop();
   * --------------------------------------------------------
   * Removes the top element of the min-heap.
   */
  void pop();

  /* void merge(BinomialHeap& other);
   * Usage: one.merge(two);
   * --------------------------------------------------------
   * Merges the contents of two BinomialHeaps into this
   * BinomialHeap.  The other heap is destructively modified
   * and emptied.
   */
  void merge(BinomialHeap& other);

  /* size_t size() const;
   * bool   empty() const;
   * Usage: while (!myHeap.empty()) { ... }
   * --------------------------------------------------------
   * Returns the number of elements in the heap and whether the
   * heap is empty, respectively.
   */
  size_t size() const;
  bool empty() const;

  /* void swap(BinomialHeap& other);
   * Usage: one.swap(two);
   * --------------------------------------------------------
   * Exchanges the contents of this heap and another heap.
   */
  void swap(BinomialHeap& other);

private:
  /* List of all the trees, by order.  If there is no tree of the given order,
   * there will be a null element in the vector.
   */
  std::vector<detail::BinomialNode<T>*> mTrees;

  /* The number of elements, cached for efficiency. */
  size_t mSize;
};

/******** Implementation Below This Point ***********/

/* Definition of the BinomialNode class and assorted operations on it. */
namespace detail {

  /* A node in a binomial tree.  Each node stores a pointer to its first 
   * child and to its rightmost sibling.
   */
  template <typename T> struct BinomialNode {
    T mValue;
    BinomialNode* mRight; // Right sibling
    BinomialNode* mChild; // Child node

    /* Constructs a BinomialNode given its value, right sibling, and child. */
    BinomialNode(const T& value, BinomialNode* right, BinomialNode* child) {
      mValue = value;
      mRight = right;
      mChild = child;
    }
  };

  /* To find the least element, we scan the tops of all of the trees in the
   * heap and return the smallest value.  This requires the use of a helper
   * function.
   *
   * Because some trees may be NULL, this comparison first checks if the either
   * tree is NULL.  If so, that tree is considered "heavier" than the other 
   * tree.  That is, the comparison places all NULL elements after all 
   * non-NULL elements.
   */
  template <typename T>
  bool CompareNodesByValue(const BinomialNode<T>* lhs, 
                           const BinomialNode<T>* rhs) {
    /* If either of the trees is null, put the non-null tree in front of the
     * null tree.
     */
    if (!lhs || !rhs)
      return !lhs < !rhs;

    /* Otherwise do a straight comparison of the values. */
    return lhs->mValue < rhs->mValue;
  }

  /* Utility function which, given two binomial trees obeying the min-heap 
   * property, merges them together into one tree and returns it as the result.
   */
  template <typename T>
  BinomialNode<T>* MergeTrees(BinomialNode<T>* lhs, BinomialNode<T>* rhs) {
    /* Check that the rhs isn't bigger and, if it is, swap the two so that
     * lhs <= rhs.
     */
    if (rhs->mValue < lhs->mValue)
      std::swap(lhs, rhs);

    /* Because we are assuming these trees are roots, the pointer rewiring is
     * not particularly tricky.  We change rhs's right pointer (currently 
     * empty) to be lhs's first child, and then retarget lhs's child pointer
     * to be rhs.
     */
    rhs->mRight = lhs->mChild;
    lhs->mChild = rhs;

    /* Return whichever one is now the root. */
    return lhs;
  }

  /* Utility function which, given two lists of BinomialTrees, merges those 
   * trees together.
   *
   * This function destructively modifies lhs and rhs by assigning lhs the 
   * result and emptying rhs.
   *
   * Binomial heap merging is very similar to addition of binary numbers.  
   * Because for each order there's either a tree of that order present or 
   * there isn't, we can think of a binomial heap as a binary number where 
   * each bit is 0 if a binomial tree of the proper order is missing and 1
   * otherwise.  When merging two heaps, we essentially "add" the two numbers
   * together using the following math:
   *
   * The sum of two empty trees is an empty tree (0 + 0 = 0)
   * The sum of an empty tree and a nonempty tree is a nonempty tree (0+1 = 1)
   * The sum of two nonempty trees is a merge of those trees, which has size 
   *     twice as large as the original tree (1+1 = 10b)
   *
   * The logic to implement this code works as follows.  We iterate across the
   * trees from lowest-order to highest-order, summing them as we go and 
   * writing the result bit by bit to some output list of trees.  At each step
   * we maintain a "carry" which holds the overflow from the previous step, 
   * if there was one.  This is analogous to the carrying performed in 
   *grade-school addition.
   */
  template <typename T>
  void BinomialHeapMerge(std::vector<BinomialNode<T>*>& lhs, 
                         std::vector<BinomialNode<T>*>& rhs) {
    /* vector to hold the result.  We use auxiliary scratch space so that we
     * don't end up with weirdness from modifying the lists as we traverse 
     * them.
     */
    std::vector<BinomialNode<T>*> result;

    /* As a simplification, we'll ensure that the two lists have the same 
     * size by padding each with null elements until they're the same size.
     */
    const size_t maxOrder = std::max(lhs.size(), rhs.size());
    lhs.resize(maxOrder);
    rhs.resize(maxOrder);

    /* Merging two binomial heaps is similar to adding two binary numbers.  
     * We proceed from the "least-significant tree" to the "most-significant 
     * tree", merging the two trees and storing the result either back in the
     * same slot (if no trees were added) or in a carry register to be used in
     * the next computation.  This next variable declaration contains the 
     * carry.
     */
    BinomialNode<T>* carry = NULL;

    /* Start marching! */
    for (size_t order = 0; order < maxOrder; ++order) {
      /* There are eight possible combinations of the nullity of the carry, 
       * lhs, and rhs trees.  To make the logic simpler, we'll add them all to
       * a temporary buffer and proceed from there.
       */
      std::vector<BinomialNode<T>*> trees;
      if (carry)
        trees.push_back(carry);
      if (lhs[order])
        trees.push_back(lhs[order]);
      if (rhs[order])
        trees.push_back(rhs[order]);
      
      /* There are now four cases to consider. */

      /* Case one: both trees and the carry are null.  Then the result of
       * this step is null and the carry should be cleared.
       */
      if (trees.empty()) {
        result.push_back(NULL);
        carry = NULL;
      }
      /* Case two: There's exactly one tree.  Then the result of this
       * step is that tree and the carry is cleared.
       */
      else if (trees.size() == 1) {
        result.push_back(trees[0]);
        carry = NULL;
      }
      /* Case three: There's exactly two trees.  Then the result of this
       * operation is NULL and the carry will be set to the merge of those
       * trees.
       */
      else if (trees.size() == 2) {
        result.push_back(NULL);
        carry = MergeTrees(trees[0], trees[1]);
      }
      /* Case four: There's exactly three trees.  Then we'll arbitrarily
       * store one of them in the current slot, then put the merge of the
       * other two into the carry.
       */
      else {
        result.push_back(trees[0]);
        carry = MergeTrees(trees[1], trees[2]);
      }
    }

    /* Finally, if the carry is set, append it to the result. */
    if (carry)
      result.push_back(carry);

    /* Clear out the rhs and assign the lhs the value of result. */
    rhs.clear();
    lhs = result;
  }

  /* Helper function to recursively destroy a binomial tree. */
  template <typename T>
  void DestroyBinomialTree(BinomialNode<T>* root) {
    if (!root) return;

    /* Clean up its siblings. */
    DestroyBinomialTree(root->mRight);

    /* Clean up its children. */
    DestroyBinomialTree(root->mChild);

    /* Destroy it. */
    delete root;
  }

  /* Helper function to recursively clone a binomial tree. */
  template <typename T>
  BinomialNode<T>* CloneBinomialTree(BinomialNode<T>* root) {
    /* Trivial to clone the empty tree. */
    if (!root) return NULL;

    /* Clone its right node and child. */
    return new BinomialNode<T>(root->mValue,
                               CloneBinomialTree(root->mRight),
                               CloneBinomialTree(root->mChild));
  }
}

/* Newly-constructed heaps are empty. */
template <typename T> 
BinomialHeap<T>::BinomialHeap() {
  mSize = 0;
}

/* Destructor cleans up all the trees. */
template <typename T>
BinomialHeap<T>::~BinomialHeap() {
  std::for_each(mTrees.begin(), mTrees.end(), detail::DestroyBinomialTree<T>);
}

/* Copy constructor clones each tree. */
template <typename T>
BinomialHeap<T>::BinomialHeap(const BinomialHeap& other) {
  mSize = other.mSize;
  for (size_t i = 0; i < mSize; ++i)
    mTrees.push_back(detail::CloneBinomialTree(other.mTrees[i]));
}

/* Assignment operator written using copy-and-swap. */
template <typename T>
BinomialHeap<T>& BinomialHeap<T>::operator = (const BinomialHeap<T>& other) {
  BinomialHeap copy(other);
  swap(copy);
  return *this;
}

/* Swap is an element-by-element swap. */
template <typename T>
void BinomialHeap<T>::swap(BinomialHeap& other) {
  mTrees.swap(other.mTrees);
  std::swap(mSize, other.mSize);
}

/* Size query just looks up the cached value. */
template <typename T>
size_t BinomialHeap<T>::size() const {
  return mSize;
}

/* empty checks whether the size is zero. */
template <typename T>
bool BinomialHeap<T>::empty() const {
  return size() == 0;
}

/* To find the top (the minimum element), we do a linear scan of the trees 
 * looking for the least value.
 */
template <typename T>
const T& BinomialHeap<T>::top() const {
  /* This may seem a bit strange.  std::min_element returns an iterator to the
   * tree with the smallest root, which mimics a BinomialNode<T>**.  The first
   * dereference gets us back a BinomialNode<T>*, and the arrow selects its
   * value.
   */
  return (*std::min_element(mTrees.begin(), mTrees.end(), 
                            detail::CompareNodesByValue<T>))->mValue;
}

/* Adding a new element to a binomial heap is just merging it with a 
 * one-element tree.
 */
template <typename T>
void BinomialHeap<T>::push(const T& value) {
  /* Create a new list of trees holding this singleton. */
  std::vector<detail::BinomialNode<T>*> singleton;
  singleton.push_back(new detail::BinomialNode<T>(value, NULL, NULL));

  /* Merge with our trees. */
  detail::BinomialHeapMerge(mTrees, singleton);

  /* Update size. */
  ++mSize;
}

/* Merging two trees uses the above helper function. */
template <typename T>
void BinomialHeap<T>::merge(BinomialHeap& other) {
  detail::BinomialHeapMerge(mTrees, other.mTrees);

  /* Update our size and the size of the other tree. */
  mSize += other.mSize;
  other.mSize = 0;
}

/* Dequeuing the min element consists of:
 * 1. Locating it.
 * 2. Breaking its children apart into a collection of trees.
 * 3. Merging those trees in with this one.
 */
template <typename T>
void BinomialHeap<T>::pop() {
  /* Locate the smallest element. */
  typename std::vector<detail::BinomialNode<T>*>::iterator minElem = 
    std::min_element(mTrees.begin(), mTrees.end(), 
                     detail::CompareNodesByValue<T>);

  /* Build up a list of its direct children. */
  std::vector<detail::BinomialNode<T>*> children;
  for (detail::BinomialNode<T>* child = (*minElem)->mChild; 
       child != NULL; child = child->mRight)
    children.push_back(child);

  /* Free the memory from the tree we just removed, then remove it
   * from the list of trees.
   */
  delete *minElem;
  *minElem = NULL;

  /* Shrink forest size if we just got rid of the last tree. */
  if (minElem == mTrees.end() - 1)
    mTrees.pop_back();

  /* Merge this list back in. */
  detail::BinomialHeapMerge(mTrees, children);

  /* Track our size. */
  --mSize;
}

#endif
