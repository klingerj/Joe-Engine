#include <malloc.h>

#include "MemAllocUtils.h"

namespace JoeEngine {
    namespace MemAllocUtils {
        void* alignedAlloc(size_t alignment, size_t size) {
            #ifdef JOE_ENGINE_PLATFORM_WINDOWS
            return _aligned_malloc(size, alignment);
            #elif JOE_ENGINE_PLATFORM_APPLE
            void* buf;
            posix_memalign(&buf, alignment, size);
            return buf;
            #endif
        }
    }
}
