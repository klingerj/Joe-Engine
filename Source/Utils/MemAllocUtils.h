#pragma once

#include "JoeEngineConfig.h"

namespace JoeEngine {
    namespace MemAllocUtils {
        //! Aligned allocation
        /*! Cross-platform function for making aligned memory allocations. */
        void* alignedAlloc(size_t alignment, size_t size);
    }
}
