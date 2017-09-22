#ifndef HAZARD_MANAGER
#define HAZARD_MANAGER

#include <cassert>

//#define DEBUG //Indicates that the at() function is used for array accesses

//Thread local id
//Note: __thread is GCC specific
extern __thread unsigned int thread_num;

#include <list>
#include <array>
#include <algorithm>
#include <iostream>

/*!
 * A manager for Hazard Pointers manipulation. 
 * \param Node The type of node to manage. 
 * \param Threads The maximum number of threads. 
 * \param Size The number of hazard pointers per thread. 
 * \param Prefill The number of nodes to precreate in the queue.
 */
template<typename Node, unsigned int Threads, unsigned int Size = 2, unsigned int Prefill = 50>
class HazardManager {
    public:
        HazardManager(){}
        ~HazardManager(){}

        HazardManager(const HazardManager& rhs) = delete;
        HazardManager& operator=(const HazardManager& rhs) = delete;

        /*!
         * Release the node. 
         */
        void releaseNode(Node* node){}

        /*!
         * \brief Release the node by checking first if it is not already in the queue. 
         * This method can be slow depending on the number of nodes already released. 
         * \param node The node to release. 
         */
        void safe_release_node(Node* node){}

        /*!
         * Return a free node for the calling thread. 
         * \return A free node
         */
        Node* getFreeNode(){return new Node;}

        /*!
         * Publish a reference to the given Node using ith Hazard Pointer. 
         * \param node The node to be published
         * \param i The index of the pointer to use. 
         */
        void publish(Node* node, unsigned int i){}

        /*!
         * Release the ith reference of the calling thread. 
         * \param i The reference index. 
         */
        void release(unsigned int i){}

        /*!
         * Release all the hazard points of the calling thread. 
         */
        void releaseAll(){}

    private:
        /* Verify the template parameters */
        static_assert(Threads > 0, "The number of threads must be greater than 0");
        static_assert(Size > 0, "The number of hazard pointers must greater than 0");
};



#endif
