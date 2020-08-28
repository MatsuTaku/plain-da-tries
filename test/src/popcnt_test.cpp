//
// Created by 松本拓真 on 2019/11/06.
//

#include "gtest/gtest.h"

#include "bo/popcnt.hpp"

namespace {

constexpr int N = 1<<16;

uint64_t rand64() {
  return uint64_t(random()) | (uint64_t(random()) << 32);
}

}

TEST(Popcnt, 64) {
  for (int i = 0; i < N; i++) {
    uint64_t x = rand64();
    int cnt = 0;
    for (int j = 0; j < 64; j++) {
      if (x & (1ull<<j))
        cnt++;
    }

    EXPECT_EQ(bo::popcnt_u64(x), cnt);
  }
}

TEST(Popcnt, 32) {
  for (int i = 0; i < N; i++) {
    uint32_t x = random();
    int cnt = 0;
    for (int j = 0; j < 32; j++) {
      if (x & (1u<<j))
        cnt++;
    }

    EXPECT_EQ(bo::popcnt_u32(x), cnt);
  }
}

TEST(Popcnt, 16) {
  for (int i = 0; i < N; i++) {
    uint16_t x = random() % (1u<<16);
    int cnt = 0;
    for (int j = 0; j < 16; j++) {
      if (x & (1u<<j))
        cnt++;
    }

    EXPECT_EQ(bo::popcnt_u16(x), cnt);
  }
}

TEST(Popcnt, 8) {
  for (int i = 0; i < N; i++) {
    uint8_t x = random() % (1u<<8);
    int cnt = 0;
    for (int j = 0; j < 8; j++) {
      if (x & (1u<<j))
        cnt++;
    }

    EXPECT_EQ(bo::popcnt_u8(x), cnt);
  }
}
