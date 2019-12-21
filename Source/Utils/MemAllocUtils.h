#pragma once

#include "JoeEngineConfig.h"

namespace JoeEngine {
    namespace MemAllocUtils {
        //! Aligned allocation
        /*!
          Cross-platform function for making aligned memory allocations.
          \param alignment the specified alignment for the allocation.
          \param size how large a buffer to allocate.
          \return pointer to the newly allocated buffer.
        */
        void* alignedAlloc(size_t alignment, size_t size);
    }
}
