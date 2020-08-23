#include "plain_da.hpp"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
  if (argc < 1) {
    std::cerr << "Usage: " << argv[0] << " [keyset]" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::ifstream ifs(argv[1]);
  if (!ifs) {
    std::cerr << argv[1] << " is not found!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::vector<std::string> keyset;
  for (std::string key; std::getline(ifs, key); ) {
    keyset.push_back(key);
  }

  plain_da::PlainDa<0> plain_da(keyset);
  for (auto& key : keyset) {
    bool ok = plain_da.contains(key);
    if (!ok) {
      std::cerr << key << "\t is not contained!" << std::endl;
    }
  }

  return 0;
}
