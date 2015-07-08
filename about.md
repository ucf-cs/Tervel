---
layout: tervel_about
---

# Tervel

Tervel is a framework to develop concurrent algorithms in.
This framework provides a memory management API and a progress assurance scheme.
The memory management API allows the developer to focus on algorithm level logic as opposed to managing the freeing or reuse of shared temporary objects.
For algorithms that exhibit complex dependencies between objects, this API provides a mechanism to express this.
The progress assurance scheme allows algorithms which are normally prone to thread starvation to achieve wait-free behavior.
It is based on Herlihy's announcement table, Kogan's method of checking it, and Feldman's association methodology
A list of publications used to construct this library is provided on our publication page.

Tervel is also a library of wait-free containers and algorithms
. These algorithms provide similar API to those found in C++'s Standard Template Library, with only slight changes to clarify the behavior of an operation in a concurrent environment.
For example, the vector data structure does not provide a random access write, but rather a random access compare and exchange function.
This allows application developers to accurately reason about the state of the system before and after the operation.

# Tervel's Purpose

Tervel's purpose is to satisfy the need for non-blocking and wait-free algorithms.
This need is driven by the rapid rate by which the number of cores in systems is increasing.
It is expected that the number of cores in a system to increase by 100 fold over the next decade.
The design principles used in the development of non-blocking algorithms, such as those found in Tervel, allow for applications to achieve a fine grain level of synchronization between threads.
Tervel provides an opportunity for designers and developers to take advantage of these fine-grained synchronization techniques.