#include "plain_da.hpp"

#include <iostream>
#include <fstream>
#include <chrono>

#include "keyset.hpp"

template <class Fn>
double ProcessTime(Fn fn) {
  auto start = std::chrono::high_resolution_clock::now();
  fn();
  auto now = std::chrono::high_resolution_clock::now();
  return (double) std::chrono::duration_cast<std::chrono::microseconds>(now-start).count();
}

template <class Da>
void Benchmark(const plain_da::KeysetHandler& keyset, const plain_da::RawTrie& trie) {
  Da plain_da;
  auto construction_time = ProcessTime([&] {
    plain_da.Build(trie);
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
  std::cout << "lookup_time: \t" << lookup_time/trie.size() << " µs/key" << std::endl;
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
  plain_da::KeysetHandler keyset(ifs);
  plain_da::RawTrie trie(keyset);

  std::cout << "- Empty-Link method" << std::endl;
  Benchmark<plain_da::PlainDa<0>>(keyset, trie);
  std::cout << "- Bit-parallelism" << std::endl;
  Benchmark<plain_da::PlainDa<1>>(keyset, trie);
  std::cout << "- Bit-parallelism + Empty-Link" << std::endl;
  Benchmark<plain_da::PlainDa<2>>(keyset, trie);

  return 0;
}
