#include <iostream>

int main() {
  using std::cout, std::endl;
  for (int i = 0; i < 8; i++) {
    cout << "{" << endl;
    for (int m = 0; m < 0x100; m++) {
      if (m % 16 == 0) {
        cout << "  ";
      }
      int c = 0;
      int a = 8;
      for (int t = 0; t < 8; t++) {
        if (m & (1<<t))
          c++;
        if (c == i+1) {
          a = t;
          break;
        }
      }
      cout << a << ", ";
      if (m % 16 == 15) {
        cout << endl;
      }
    }
    cout << "}," << endl;
  }
}
