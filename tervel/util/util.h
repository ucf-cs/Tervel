#ifndef TERVEL_UTIL_H_
#define TERVEL_UTIL_H_

namespace tervel {
namespace util {

/**
 * Returns whether or not the passed value is has one of the reserved bits set
 * to 1.
 *
 * TODO(steven): implement this.
 */
inline bool isValid(void * value) {
  return true;
}

}  // namespace util
}  // namespace tervel

// A macro to disallow the copy constructor and operator= functions.  This
// should be used in the `private` declarations for a class. Use unless you have
// a good reason for a class to be copy-able.
// see:
//   http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Copy_Constructors
//   http://stackoverflow.com/questions/20026445
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
  TypeName(const TypeName&) = delete;       \
  void operator=(const TypeName&) = delete

#endif  // TERVEL_UTIL_H_
