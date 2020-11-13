#include "plain_da.hpp"

#include <iostream>
#include <fstream>
#include <chrono>

#include "keyset.hpp"

namespace {

template <class Fn>
double ProcessTime(Fn fn) {
  auto start = std::chrono::high_resolution_clock::now();
  fn();
  auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::micro>(now-start).count();
}

constexpr int BenchKeyCounts = 1000000;
constexpr int LoopTimes = 10;

template <class Da>
void Benchmark(const plain_da::KeysetHandler& keyset, const plain_da::RawTrie& trie) {
  Da plain_da;
  auto construction_time = ProcessTime([&] {
    plain_da.Build(trie);
  });
  std::cout << "construction_time: \t" << construction_time/1000000 << " s" << std::endl;

  { // Check is construction perfect.
    for (auto &key : keyset) {
      bool ok = plain_da.contains(key);
      if (!ok) {
        std::cout << "ERROR! " << key << "\t is not contained!" << std::endl;
        return;
      }
    }
  }

  std::vector<std::string> bench_keyset(BenchKeyCounts);
  for (int i = 0; i < BenchKeyCounts; i++) {
    bench_keyset[i] = keyset[random()%keyset.size()];
  }
  auto bench_for_random_keys = [&] {
    for (auto &key : bench_keyset) {
      bool ok = plain_da.contains(key);
      if (!ok) {
        std::cout << "ERROR! " << key << "\t is not contained!" << std::endl;
        return;
      }
    }
  };
  { // Warm up
    bench_for_random_keys();
  }
  auto lookup_time = ProcessTime([&] {
    for (int i = 0; i < LoopTimes; i++) {
      bench_for_random_keys();
    }
  });
  std::cout << "lookup_time: \t" << lookup_time/trie.size()/LoopTimes << " µs/key" << std::endl;
}

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

//  std::cout << "- Empty-Link method" << std::endl;
//  Benchmark<plain_da::PlainDa<plain_da::da_construction_type_ELM, false>>(keyset, trie);
  std::cout << "- Bit-parallelism" << std::endl;
  Benchmark<plain_da::PlainDa<plain_da::da_construction_type_WW, false>>(keyset, trie);
  std::cout << "- Bit-parallelism + EdgeOrdering" << std::endl;
  Benchmark<plain_da::PlainDa<plain_da::da_construction_type_WW, true>>(keyset, trie);
//  std::cout << "- Bit-parallelism + Empty-Link" << std::endl;
//  Benchmark<plain_da::PlainDa<plain_da::da_construction_type_WW_ELM, false>>(keyset, trie);

  return 0;
}
