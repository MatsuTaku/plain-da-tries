//
// Created by 松本拓真 on 2019/11/06.
//

#include "gtest/gtest.h"

#include "bo/select.hpp"

namespace {

constexpr int N = 1<<16;

uint64_t rand64() {
  return uint64_t(random()) | (uint64_t(random()) << 32);
}

}

TEST(Select, 64) {
  for (int i = 0; i < N; i++) {
    uint64_t x = rand64();
    int cnt = 0;
    for (int j = 0; j < 64; j++) {
      if (x & (1ull<<j)) {
        auto s = bo::select_u64(x, cnt);
        EXPECT_EQ(s, j);
        if (s != j) {
          auto bin_show = [](uint64_t x) {
            std::string s;
            for (int i = 0; i < 64; i++) {
              s.push_back((x&(1ull<<i)) ? '1' : '0');
            }
            return s;
          };
          std::cerr << bin_show(x) << " " << cnt << " " << s << " " << j << std::endl;
        }
        cnt++;
      }
    }
  }
}
