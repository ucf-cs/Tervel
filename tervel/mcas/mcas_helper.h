// REVIEW(carlos): should be TERVEL_*_H_
#ifndef UCF_MCAS_HELPER_
#define UCF_MCAS_HELPER_

// REVIEW(carlos): include paths should start w/ tervel
#include "mcas_casrow.h"
#include "mcas.h"
// REVIEW(carlos): should be tervel/memory/descriptor.h
#include "thread/descriptor.h"



// REVIEW(carlos): should be namespace tervel
namespace ucf {
namespace mcas {

template<class T>
// REVIEW(carlos): no such class as thread::Descriptor (it's memory::Descriptor)
// REVIEW(carlos): space on both sides of colon
// REVIEW(carlos): class name is redundant given that namespace name is mcas
class MCASHelper: public thread::Descriptor {
// REVIEW(carlos): Indentation is wonky in this class
// REVIEW(carlos): class commentg goes above class name
/**
 * This class is the MCAS operation's helper. The Helper or MCH is used to 
 * replace the expected value of the specified address. 
 * 
 */

// REVIEW(carlos): This won't do what you expect it to do. You can't define
//   the body of a templated class/function in a cc file and compile it
//   seperately. It has to be in an included header.
typedef CasRow<T> t_CasRow;
typedef MCASHelper<T> t_MCASHelper;
typedef MCAS<T> t_MCAS;

// REVIEW(carlos): public declaration should be at top of class
// REVIEW(carlos): indentation should be 1 space for the public: header, 2
//   spaces for everything else
  public:
    /**
     * @param mcas_op the t_MCAS which contains the referenced cas_row
     * @param cas_row the referenced row in the t_MCAS.
     */
    MCASHelper<T>(t_MCAS *mcas_op, t_CasRow *cas_row)
        : cas_row_(cas_row), mcas_op_(mcas_op) {}

    /**
     * This function is called after this objects rc count was incremented.
     * It acquires a temporary HP watch on the MCAS op (via last_row_), ensures
     * it is is associated, and if so returns true.
     * If it is not associated or it was removed, it returns false
     *
     * @param address the address this MCH was read from
     * @param value the bitmarked value of this MCH
     * @return returns whether or not the watch was successfull.
     */
    bool on_watch(std::atomic<void *> *address, T value);

    /** 
     * This function is called to remove an MCH by completing its associated
     * operation.
     *
     * @params address the location where the MCH resides
     * @params value the value of this MCH as it was read at the address
     * @return the current value of the address
     */
    void * complete(std::atomic<void *> *address, void * value);

    /** 
     * This function is called to remove a descriptor which is in conflict with
     * a MCAS operation. If the descriptor is a MCH, then a check is performed
     * to determine if it is for the same operation. If so, then it is treated 
     * differently. Otherwise, the descriptors complete function is called.
     *
     * @params address the location where a descriptor was read from
     * @params value the value that was read which is a descriptor
     * @params last_row an identifier of the current MCAS operation 
     * @return the current value of the address
     */
    // REVIEW(carlos): excess vertical whitespace

    static T mcas_remove(std::atomic<T> *address, T value, t_CasRow *last_row);

 private:
  // The Row in the MCAS operation this MCH was placed for
  t_CasRow *cas_row_;
  // The MCAS which contains the cas_row_
  t_MCAS *mcas_op_;
};

}  // End mcas namespace
// REVIEW(carlos): should be namespace tervel
}  // End ucf nampespace
// REVIEW(carlos): should be TERVEL_*_H_
// REVIEW(carlos): should be a space between closing brace and close of
//   include guard
#endif  // UCF_MCAS_HELPER_
