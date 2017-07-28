---
layout: tervel_documentation
---

# Tutorial

Tervel employs a collection of techniques to allow for non-blocking synchronization in the form of lock-freedom and wait-freedom. Lock-freedom guarantees that at least one thread will make progress in a finite amount of time. Wait-freedom guarantees that all threads make progress in a finite amount of time. 

This tutorial will detail the overall steps for including Tervel in an existing application, as well as how to use Tervel to guarantee lock-freedom or wait-freedom in an existing algorithm.

Note: 	For many lock-free applications, it may not be necessary to employ the progress assurance scheme or operation records. However, in most wait-free designs these implementations will be required to guarantee system-wide progress in a finite number of steps.  

## Including Tervel in existing application

Tervel uses descriptor-based techniques for thread synchronization. Assuming an already descriptor-based algorithm, the use of these descriptor objects influences whether RC or HP memory protection may be appropriate. For help implementing descriptors, refer to the "Descriptor" section.

RC Element
: We recommend extending the `tervel::memory::rc::PoolElement` class for small, short-lived objects that are used repeatedly. 

HP Element
: We recommend extending the `tervel::memory::hp::Element` class for very large or varying size objects that are used infrequently. 

For reference on how to use the memory protection, refer to the `Stack::Accessor` class in `/tervel/containers/lf/`. In the lock-free stack example, the `Stack::Node` class extends the 'hp::Element` class. `Accessor::load()` retrieves an element from an address and calls `HazardPointer::watch()` on it's address. Every time the top of the stack is read, `Accessor::load()` must be called to retrieve the element and attempt to watch the address. 

## Descriptor

A descriptor class is provided to guide in implementing descriptor objects. Objects extending this class must include `/tervel/util/descriptor.h`

Any object that extends `tervel::util::Descriptor` must implement the `complete()` and `get_logical_value()` methods. Refer to `/tervel/util/descriptor.h` for the functional prototypes.

The `complete()` method guarantees upon return that the descriptor no longer exists at the address it was placed.

The `get_logical_value()` method returns the value of that object at the descriptor's address. 

Additionally, you may choose to implement the `on_watch()`, `on_unwatch()`, and `on_is_watched()` methods. These methods are optional, and only required in cases where there is a dependency between descriptor objects. 

## Progress Assurance

The progress assurance scheme uses an announcement table to guarantee progress. In Tervel, we refer to an announcement table as an operation record. An operation record is a type of descriptor object that contains information necessary for an arbitrary thread to execute an entire operation. 

For reference on how to use the progress assurance scheme, refer to the Stack class in `/tervel/containers/wf/stack_imp.h`. This is the wait-free version of the same stack implemented in `/tervel/containers/lf/`. The wait-free `Stack::Node` object still extends the `hp::Element` class. 

At the start of each operation, a thread checks the announcement table for other operations that need help by calling `ProgressAssurance::check_for_announcement()`. If no thread needs help, the operation proceeds similar to the lock-free implementation, with the exception of the Limit object, `progAssur`. Every time the calling thread attempts to perform it's operation and fails, it calls `progAssur.isDelayed()`. Once these failures reach a certain limit, the thread adds it's operation descriptor to the announcement table by calling `ProgressAssurance::make_announcment()`. This action will result in one or more threads attempting to help the current thread's operation complete. 

## Operation Record

An Operation Record is required for wait-freedom, but may be omitted if lock-freedom is all that is desired. An Operation Record must be implemented in each operation that contains an unbounded loop, or a loop that may execute indefinitely under certain conditions. 

To implement an Operation Record, you may extend the `tervel::util::OpRecord` class. 

The `help_complete()` method must be implemented. This function is called by a thread in order to help a different thread complete it's operation. It must be guaranteed that upon return of this function, the operation described by the `OpRecord` is complete. 

An Operation Record can be used to create a bound on unbounded loops by ensuring that after a finite amount of steps, all threads will be attempting to help complete the operation. 