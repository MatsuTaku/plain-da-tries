#include <iostream>

int main() {
  std::cout << "{" << std::endl;
  for (size_t i = 0; i < 256; i++) {
    if (i % 16 == 0)
      std::cout << "  ";
    size_t cnt = 0;
    for (size_t j = 0; j < 8; j++) {
      if ((i >> j) & 1)
        cnt++;
    }
    std::cout << cnt << ", ";
    if (i % 16 == 15)
      std::cout << std::endl;
  }
  std::cout << "}" << std::endl;

}
