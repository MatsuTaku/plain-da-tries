//
// Created by 松本拓真 on 2019/11/06.
//

#include "gtest/gtest.h"

#include "bo/bextr.hpp"

namespace {

constexpr int N = 1<<16;

uint64_t rand64() {
  return uint64_t(random()) | (uint64_t(random()) << 32);
}

}

TEST(Bextr, 64) {
  for (int i = 0; i < N; i++) {
    auto val = rand64();
    int s = random() % 64;
    int l = random() % (64 - s) + 1;
    uint64_t bar = (l < 64) ? (1ull<<l)-1 : -1;
    uint64_t mask = bar << s;

    EXPECT_EQ(bo::bextr_u64(val, s, l), (val & mask) >> s);
  }
}
