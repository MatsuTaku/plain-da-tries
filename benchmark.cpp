#include "plain_da.hpp"

#include <iostream>
#include <fstream>
#include <chrono>

#include "keyset.hpp"
#include "double_array_base.hpp"

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
void Benchmark(const plain_da::KeysetHandler& keyset, const plain_da::RawTrie& trie, const plain_da::KeysetHandler& bench_keyset) {
  Da plain_da;
  auto construction_time = ProcessTime([&] {
    plain_da.Build(trie);
  });
  std::cout << "construction_time: \t" << construction_time/1000000 << " s" << std::endl;

  { // Check is construction perfect.
    std::cout << "Test..." << std::endl;
    for (auto &key : keyset) {
      bool ok = plain_da.contains(key);
      if (!ok) {
        std::cout << "NG" << std::endl;
        std::cout << "ERROR! " << key << "\t is not contained!" << std::endl;
        return;
      }
    }
    std::cout << "OK" << std::endl;
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
  std::cout << "lookup_time: \t" << lookup_time/BenchKeyCounts/LoopTimes << " µs/key" << std::endl << std::endl;
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
  plain_da::KeysetHandler bench_keyset;
  for (int i = 0; i < BenchKeyCounts; i++)
    bench_keyset.insert(keyset[random()%keyset.size()]);
  bench_keyset.update_list();

//  std::cout << "- EmptyLink" << std::endl;
//  Benchmark<plain_da::PlainDaTrie<plain_da::da_xor_operation_tag, plain_da::da_construction_type_ELM, false>>(keyset, trie, bench_keyset);
//  std::cout << "- EmptyLink + EdgeOrdering" << std::endl;
//  Benchmark<plain_da::PlainDaTrie<plain_da::da_xor_operation_tag, plain_da::da_construction_type_ELM, true>>(keyset, trie, bench_keyset);
//  std::cout << "- BitParallelism" << std::endl;
//  Benchmark<plain_da::PlainDaTrie<plain_da::da_xor_operation_tag, plain_da::da_construction_type_WW, false>>(keyset, trie, bench_keyset);
//  std::cout << "- BitParallelism + EdgeOrdering" << std::endl;
//  Benchmark<plain_da::PlainDaTrie<plain_da::da_xor_operation_tag, plain_da::da_construction_type_WW, true>>(keyset, trie, bench_keyset);
//  std::cout << "- BitParallelism + Empty-Link" << std::endl;
//  Benchmark<plain_da::PlainDaTrie<plain_da::da_construction_type_WW_ELM, false>>(keyset, trie, bench_keyset);

  std::cout << "- MP+ - EmptyLink" << std::endl;
  Benchmark<plain_da::PlainDaMpTrie<
      plain_da::DoubleArrayBase<
          plain_da::da_plus_operation_tag,
          plain_da::da_construction_type_ELM
          >,
      false
      >>(keyset, trie, bench_keyset);
  std::cout << "- MP+ - BitParallelism" << std::endl;
  Benchmark<plain_da::PlainDaMpTrie<
      plain_da::DoubleArrayBase<
          plain_da::da_plus_operation_tag,
          plain_da::da_construction_type_WW
      >,
      false
  >>(keyset, trie, bench_keyset);
  std::cout << "- MP+ - BitParallelism + Empty-Link" << std::endl;
  Benchmark<plain_da::PlainDaMpTrie<
      plain_da::DoubleArrayBase<
          plain_da::da_plus_operation_tag,
          plain_da::da_construction_type_WW_ELM
          >,
      false
      >>(keyset, trie, bench_keyset);

  return 0;
}
