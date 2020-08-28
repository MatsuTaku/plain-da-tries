#include <iostream>

int main() {
  std::cout << "{" << std::endl;
  for (int i = 0; i < 256; i++) {
    if (i%16 == 0)
      std::cout << "  ";
    int j = i;
    j = ((j&0x55)<<1) | ((j>>1)&0x55);
    j = ((j&0x33)<<2) | ((j>>2)&0x33);
    j = ((j&0x0F)<<4) | ((j>>4)&0x0F);
    std::cout << j << ", ";
    if (i%16 == 15)
      std::cout << std::endl;
  }
  std::cout << "}" << std::endl;
}
