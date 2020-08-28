# libbo

C++ Header-only Library of Practical Bit Operations

## Supporting bit operations

  - ctz - count trailing zeros
  - clz - count leading zeros
  - popcnt - poplation count
  - bextr - bits extract
  - select - select nth bit
  - swapnext - swap next nth bits to each other
  
  ## LISENCE
  This library is lisenced by The Unlisence.
  
  ## Usage
  You can handle libbo only including header files.
  
  Install to your include directory by cmake as forrows:
  ```bash
  cmake .
  cmake --build .
  ctest
  cmake --install .
  ```
  
  We recommend to compile your program with option `-march=native` 
  
