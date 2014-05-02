#ifndef TERVEL_UTIL_H_
#define TERVEL_UTIL_H_

namespace tervel {
namespace util {
namespace memory {
namespace hp {
/**
   * If true, then ElementList will never delete any HP protected elements.
   * Instead, elements should be stock-piled and left untouched when they're 
   * attempted to be freed This allows the user to view associations.
   * Entirely for debug purposes.
   */
constexpr bool NO_DELETE_HP_ELEMENTS {true};
}
namespace rc {
/**
 * If true, then DescriptorPool shouldn't reuse old pool elements when being
 * asked, even if it's safe to fo so. Instead, elements should be stock-piled
 * and left untouched when they're returned to the pool. This allows the user
 * to view associations. Entirely for debug purposes.
 */
constexpr bool NO_REUSE_RC_DESCR {true};
}
}
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
