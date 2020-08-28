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

#ifndef BO_BITREVERSE_HPP_
#define BO_BITREVERSE_HPP_

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#include <cassert>

namespace bo {

constexpr uint8_t kBitreverseTable[0x100] = {
  0, 128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208, 48, 176, 112, 240,
  8, 136, 72, 200, 40, 168, 104, 232, 24, 152, 88, 216, 56, 184, 120, 248,
  4, 132, 68, 196, 36, 164, 100, 228, 20, 148, 84, 212, 52, 180, 116, 244,
  12, 140, 76, 204, 44, 172, 108, 236, 28, 156, 92, 220, 60, 188, 124, 252,
  2, 130, 66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50, 178, 114, 242,
  10, 138, 74, 202, 42, 170, 106, 234, 26, 154, 90, 218, 58, 186, 122, 250,
  6, 134, 70, 198, 38, 166, 102, 230, 22, 150, 86, 214, 54, 182, 118, 246,
  14, 142, 78, 206, 46, 174, 110, 238, 30, 158, 94, 222, 62, 190, 126, 254,
  1, 129, 65, 193, 33, 161, 97, 225, 17, 145, 81, 209, 49, 177, 113, 241,
  9, 137, 73, 201, 41, 169, 105, 233, 25, 153, 89, 217, 57, 185, 121, 249,
  5, 133, 69, 197, 37, 165, 101, 229, 21, 149, 85, 213, 53, 181, 117, 245,
  13, 141, 77, 205, 45, 173, 109, 237, 29, 157, 93, 221, 61, 189, 125, 253,
  3, 131, 67, 195, 35, 163, 99, 227, 19, 147, 83, 211, 51, 179, 115, 243,
  11, 139, 75, 203, 43, 171, 107, 235, 27, 155, 91, 219, 59, 187, 123, 251,
  7, 135, 71, 199, 39, 167, 103, 231, 23, 151, 87, 215, 55, 183, 119, 247,
  15, 143, 79, 207, 47, 175, 111, 239, 31, 159, 95, 223, 63, 191, 127, 255,
};

inline uint64_t bitreverse_u64(uint64_t x) {
  x = ((x&0x5555555555555555)<<1) | ((x&0xAAAAAAAAAAAAAAAA)>>1);
  x = ((x&0x3333333333333333)<<2) | ((x&0xCCCCCCCCCCCCCCCC)>>2);
  x = ((x&0x0F0F0F0F0F0F0F0F)<<4) | ((x&0xF0F0F0F0F0F0F0F0)>>4);
//  x = _bswap64(x);
  x = ((x&0x00FF00FF00FF00FF)<<8) | ((x&0xFF00FF00FF00FF00)>>8);
  x = ((x&0x0000FFFF0000FFFF)<<16) | ((x&0xFFFF0000FFFF0000)>>16);
  x = (x<<32) | (x>>32);
  return x;
//  return ((uint64_t)kBitreverseTable[x&0xFF]<<56 |
//	 	  (uint64_t)kBitreverseTable[(x>>8)&0xFF]<<48 |
//	 	  (uint64_t)kBitreverseTable[(x>>16)&0xFF]<<40 |
//	 	  (uint64_t)kBitreverseTable[(x>>24)&0xFF]<<32 |
//	 	  (uint64_t)kBitreverseTable[(x>>32)&0xFF]<<24 |
//	 	  (uint64_t)kBitreverseTable[(x>>40)&0xFF]<<16 |
//	 	  (uint64_t)kBitreverseTable[(x>>48)&0xFF]<<8 |
//	  	  (uint64_t)kBitreverseTable[x>>56]);
}

}

#endif //BO_BITREVERSE_HPP_
