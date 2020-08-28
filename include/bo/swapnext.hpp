/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
 */

#ifndef BO_SWAPNEXT_HPP_
#define BO_SWAPNEXT_HPP_

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace bo {

inline constexpr uint64_t swapnext1_u64(uint64_t x) {
  return ((x & 0xAAAAAAAAAAAAAAAA) >> 1) | ((x & 0x5555555555555555) << 1);
}

inline constexpr uint64_t swapnext2_u64(uint64_t x) {
  return ((x & 0xCCCCCCCCCCCCCCCC) >> 2) | ((x & 0x3333333333333333) << 2);
}

inline constexpr uint64_t swapnext4_u64(uint64_t x) {
  return ((x & 0xF0F0F0F0F0F0F0F0) >> 4) | ((x & 0x0F0F0F0F0F0F0F0F) << 4);
}

inline uint64_t swapnext8_u64(uint64_t x) {
#ifdef __MMX__

  auto xx = (__m64) x;
  return (uint64_t) _mm_or_si64(_mm_slli_pi16(xx, 8), _mm_srli_pi16(xx, 8));

#else

  return ((x & 0xFF00FF00FF00FF00) >> 8) | ((x & 0x00FF00FF00FF00FF) << 8);

#endif
}

inline uint64_t swapnext16_u64(uint64_t x) {
#ifdef __MMX__

  auto xx = (__m64) x;
  return (uint64_t) _mm_or_si64(_mm_slli_pi32(xx, 16), _mm_srli_pi32(xx, 16));

#else

  return ((x & 0xFFFF0000FFFF0000) >> 16) | ((x & 0x0000FFFF0000FFFF) << 16);

#endif
}

inline constexpr uint64_t swapnext32_u64(uint64_t x) {
  return (x >> 32) | (x << 32);
}

}

#endif //BO_SWAPNEXT_HPP_
