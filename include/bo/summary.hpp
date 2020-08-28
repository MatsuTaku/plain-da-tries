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

#ifndef BO_SUMMARY_HPP_
#define BO_SUMMARY_HPP_

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace bo {

// summarize x_{0-7} by follows:
// { 0: x = 0
//   1: 1 <= x < 256
inline uint8_t summary_u64_each8(uint64_t x) {
#ifdef __MMX__

  auto c = uint64_t(_mm_cmpeq_pi8(__m64(x), __m64(0ull)));
  c = ~c & 0x8080808080808080ull;

#else

  constexpr uint64_t hmask = 0x8080808080808080ull;
  constexpr uint64_t lmask = 0x7F7F7F7F7F7F7F7Full;
  uint64_t a = x & hmask;
  uint64_t b = x & lmask;
  b = hmask - b;
  b = ~b;
  auto c = (a | b) & hmask;

#endif
  c *= 0x0002040810204081ull;
  return c >> 56;
}

} // namespace bo

#endif //BO_SUMMARY_HPP_
