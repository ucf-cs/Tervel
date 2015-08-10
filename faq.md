---
layout: tervel_default
title: FAQ
---

What is Tervel?
-------------

Tervel is a wait-free framework and library of concurrent algorithms and containers.
Its primary goal is to ensure that each component of Tervel is wait-free.
This means that each function will return after a finite number of steps, regardless of the system scheduler or actions of other threads.


What is special about Tervel?
-----------------------

Tervel was designed to bring together the many techniques and methodologies found in literature  into a single usable framework.
Its design was heavily influenced by the challenges and requirements faced when implementing wait-free algorithms.

Its most notable features are:

* Memory Reclamation

  * shared pools

* Progress Assurance

* Inter-Thread Helping Techniques

  * Descriptor design methodologies

  * Association model

  * Integration with memory reclamation

  * Composable design patters

See the [concepts page](docs/build-ref.html) for details.



What is Tervel's origin?
-----------------------

Tervel was created to solve the problems we faced when designing and implementing wait-free algorithms.
We found that for each implementation we were re-writing or even worse copying code that was used in previous algorithms.
By constructing a framework, we found that the implementation time significantly decreased.
Additionally the number of difficult bugs, amount of coding, and testing was also significantly reduced.

By migrating our old algorithms to Tervel, it made it easier for designers to incorporate various data structures into their application. Previously they had to integrate multiple disjoint code bases.


Why would I want to use Tervel?
------------------------------

If you have requirements of wait-freedom, then Tervel is a must!
Thats really the only reason... unless you get some joy playing around with progress guarantees and concurrency, and if that is the case you should contribute.


Do I need wait-freedom?
------------------------------

If a thread stopped executing at an arbitrary point in its execution (and never resumed), whats the worst that would happen?

Is there examples?
-------------------

Yes! check out the test directory!


On what platforms does Tervel run?
---------------------------------

Anything that runs C++11 can run Tervel.




How stable is Tervel's feature set?
--------------------

Its been tested, but not exhaustively, this is an alpha release after all.



Can I contribute to the Tervel code base?
----------------------------------------

Please see our [contribution guidelines](contributing.html).



How do I contact the team?
--------------------------

You can send us messages through github or at <tervel@cse.eecs.ucf.edu>.


Where do I report bugs?
-----------------------

Send e-mail to <tervel@cse.eecs.ucf.edu>.




