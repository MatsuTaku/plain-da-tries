#include "convolution.hpp"

#include <iostream>

using namespace plain_da::convolution;

constexpr int _f[4] = {1,5,3,4};
constexpr int _g[4] = {5,2,1,3};
constexpr int expected[4] = {30,40,39,34};

int main() {
  std::cout << "Test index_xor_convolution..." << std::endl;
  int f[4], g[4];
  for (int i = 0; i < 4; i++) {
    f[i] = _f[i];
    g[i] = _g[i];
  }
  fwt(g, 4);
  index_xor_convolution_for_xcheck(f, g, 4);
  for (int i = 0; i < 4; i++) {
    if (f[i] != expected[i]) {
      std::cout << "Test failed" << std::endl;

      std::cout << "f: \t\t";
      for (int i = 0; i < 4; i++)
        std::cout << _f[i] << ' ';
      std::cout << std::endl;

      std::cout << "g: \t\t";
      for (int i = 0; i < 4; i++)
        std::cout << _g[i] << ' ';
      std::cout << std::endl;

      std::cout << "expected: \t";
      for (int i = 0; i < 4; i++)
        std::cout << expected[i] << ' ';
      std::cout << std::endl;

      std::cout << "result: \t";
      for (int i = 0; i < 4; i++)
        std::cout << f[i] << ' ';
      std::cout << std::endl;

      return 1;
    }
  }
  std::cout << "OK" << std::endl;
  
  return 0;
}
