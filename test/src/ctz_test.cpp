//
// Created by 松本拓真 on 2019/11/09.
//

#include "gtest/gtest.h"

#include "bo/ctz.hpp"

namespace {

constexpr int N = 1<<16;

uint64_t rand64() {
  return uint64_t(random()) | (uint64_t(random()) << 32);
}

}

TEST(Ctz, 64) {
  for (int i = 0; i < N; i++) {
    auto val = rand64();
    int ctz = 64;
    for (int j = 0; j < 64; j++) {
      if (val & (1ull << j)) {
        ctz = j;
        break;
      }
    }

    EXPECT_EQ(bo::ctz_u64(val), ctz);
  }
}
