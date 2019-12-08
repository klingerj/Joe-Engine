# Check for AVX/AVX2 capabilities
# Based on: https://github.com/soedinglab/hh-suite/blob/master/cmake/CheckSSEFeatures.cmake

include(CheckCXXSourceRuns)

set(JOE_ENGINE_SIMD_NONE OFF)
set(JOE_ENGINE_SIMD_AVX OFF)
set(JOE_ENGINE_SIMD_AVX2 OFF)
set(JOE_ENGINE_SIMD_AVX512 OFF)

# TODO: detect AVX512 support

check_cxx_source_runs("
      #include <immintrin.h>
      int main()
      {
        __m256i a, b;
        a = _mm256_set1_epi8 (1);
        b = a;
        _mm256_add_epi8 (a,a);
        return 0;
      }"
      HAVE_AVX2_EXTENSIONS)

if (HAVE_AVX2_EXTENSIONS)
    message(STATUS "Found AVX2 support")
    set(JOE_ENGINE_SIMD_AVX2 ON)
    if (WIN32)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/arch:AVX2")
    else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-mavx2")
        endif()
    endif()
    return()
endif()

# AVX
#check_cxx_source_runs("
#      #include <immintrin.h>
#      int main()
#      {
#        __m256 a, b;
#        float vals[8] = {1, 2, 3, 4, 5, 6, 7, 8};
#        const int mask = 123;
#        a = _mm256_loadu_ps(vals);
#        b = a;
#        b = _mm256_dp_ps (a, a, 3);
#        _mm256_storeu_ps(vals,b);
#        return 0;
#      }"
#      HAVE_AVX_EXTENSIONS)
#
#if (HAVE_AVX_EXTENSIONS)
#    set(JOE_ENGINE_SIMD_AVX2 ON)
#    message(STATUS "Found AVX support")
#    if (WIN32)
#        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/arch:AVX")
#    else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#            set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-mavx")
#        endif()
#    endif()
#    return()
#endif()

# If we made it this far past the return() calls, no SIMD support is present on this platform
set(JOE_ENGINE_SIMD_NONE ON)
