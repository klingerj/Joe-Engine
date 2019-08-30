#pragma once

#include "JoeEngineConfig.h"

namespace JoeEngine {
    namespace MemAllocUtils {
        void* alignedAlloc(size_t alignment, size_t size);
    }
}
