#include "tervel/util/info.h"

namespace tervel {

__thread ThreadContext * tl_thread_info;
__thread void * last_watch;  // TODO(steven) delete this
}  // namespace tervel
