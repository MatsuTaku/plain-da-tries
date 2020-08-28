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

#ifndef BO_CLZ_HPP_
#define BO_CLZ_HPP_

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#include <cassert>
#include "bextr.hpp"
#include "summary.hpp"

namespace bo {

// floor(log_2(x)) + 1
inline constexpr uint8_t log2p1_u8(uint8_t x) {
  if (x >= 0x80)
    return 8;

  uint64_t p = uint64_t(x) * 0x0101010101010101ull;
  p -= 0x8040201008040201ull;
  p = ~p & 0x8080808080808080ull;
  p >>= 7;
  p *= 0x0101010101010101ull;
  p >>= 56;
  return p & 0b1111;
}

// Most Significant-Set Bit
inline constexpr uint8_t mssb_u8(uint8_t x) {
  assert(x != 0);
  return log2p1_u8(x) - 1;
}

inline uint64_t clz_u64(uint64_t x) {
#ifdef __LZCNT__

  return _lzcnt_u64(x);

#else

  if (x == 0)
    return 64;
  auto i = mssb_u8(summary_u64_each8(x));
  auto j = mssb_u8(uint8_t(bextr_u64(x, i * 8, 8)));
  return 63 - (i * 8 + j);

#endif
}

} // namespace bo

#endif //BO_CLZ_HPP_
