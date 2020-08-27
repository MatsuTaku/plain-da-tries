#include "plain_da.hpp"

#include <iostream>
#include <fstream>
#include <chrono>

template <class Fn>
double ProcessTime(Fn fn) {
  auto start = std::chrono::high_resolution_clock::now();
  fn();
  auto now = std::chrono::high_resolution_clock::now();
  return (double) std::chrono::duration_cast<std::chrono::microseconds>(now-start).count();
}

template <class Da>
void Benchmark(const std::vector<std::string>& keyset) {
  Da plain_da;
  auto construction_time = ProcessTime([&] {
    plain_da.Build(keyset);
  });
  std::cout << "construction_time: \t" << construction_time/1000000 << " s" << std::endl;

  auto lookup_time = ProcessTime([&] {
    for (auto& key : keyset) {
      bool ok = plain_da.contains(key);
      if (!ok) {
        std::cout << "ERROR! " << key << "\t is not contained!" << std::endl;
        return;
      }
    }
  });
  std::cout << "lookup_time: \t" << lookup_time/keyset.size() << " µs/key" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [keyset]" << std::endl;
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

  std::cout << "- Empty-Link method" << std::endl;
  Benchmark<plain_da::PlainDa<1>>(keyset);
  std::cout << "- Bit-parallelism" << std::endl;
  Benchmark<plain_da::PlainDa<2>>(keyset);

  return 0;
}
