#include "convolution.hpp"

#include <iostream>

using namespace plain_da::convolution;

using mint = ModuloNTT;
constexpr mint _f[8] = {1,5,3,4,0,0,0,0};
constexpr mint _g[8] = {5,2,1,3,0,0,0,0};
constexpr mint expected[8] = {5,27,26,34,26,13,12,0};

int main() {
  std::cout << "Test index_sum_convolution..." << std::endl;
  mint f[8], g[8];
  for (int i = 0; i < 8; i++) {
    f[i] = _f[i];
    g[i] = _g[i];
  }

  ntt(g, 8);
  index_sum_convolution_for_xcheck(f, g, 8);
  for (int i = 0; i < 8; i++) {
    if (f[i] != expected[i]) {
      std::cout << "Test failed" << std::endl;

      std::cout << "f: \t\t\t";
      for (int i = 0; i < 8; i++)
        std::cout << _f[i].val() << ' ';
      std::cout << std::endl;

      std::cout << "g: \t\t\t";
      for (int i = 0; i < 8; i++)
        std::cout << _g[i].val() << ' ';
      std::cout << std::endl;

      std::cout << "expected: \t";
      for (int i = 0; i < 8; i++)
        std::cout << expected[i].val() << ' ';
      std::cout << std::endl;

      std::cout << "result: \t";
      for (int i = 0; i < 8; i++)
        std::cout << f[i].val() << ' ';
      std::cout << std::endl;

      return 1;
    }
  }
  std::cout << "OK" << std::endl;

  return 0;
}
