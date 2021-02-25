#ifndef PLAIN_DA_TRIES__PLAIN_DA_HPP_
#define PLAIN_DA_TRIES__PLAIN_DA_HPP_

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <deque>
#include <unordered_map>
#include <limits>
#include <cassert>
#include <iostream>
#include <bitset>
#include <chrono>
#include <iterator>
#include <numeric>
#include <algorithm>

#include "double_array_base.hpp"
#include "keyset.hpp"

namespace plain_da {

template <typename OperationTag, typename ConstructionType, bool EdgeOrdering>
class PlainDaTrie {
 public:
  using da_type = DoubleArrayBase<OperationTag, ConstructionType>;

 public:
  PlainDaTrie() = default;

  explicit PlainDaTrie(const KeysetHandler& keyset) {
    Build(keyset);
  }

  void Build(const KeysetHandler& keyset);

  explicit PlainDaTrie(const RawTrie& trie) {
    Build(trie);
  }

  void Build(const RawTrie& trie);

  size_t size() const { return bc_.size(); }

  bool contains(const std::string& key) const {
    return _contains(key);
  }
  bool contains(std::string_view key) const {
    return _contains(key);
  }
 private:
  template <typename Key>
  bool _contains(Key key) const {
    index_type idx = 0;
    for (uint8_t c : key) {
      auto nxt = bc_.Operate(bc_[idx].base, c);
      if (nxt >= bc_.size() or bc_[nxt].check != idx) {
        return false;
      }
      idx = nxt;
    }
    auto nxt = bc_.Operate(bc_[idx].base, kLeafChar);
    return !(nxt >= bc_.size() or bc_[nxt].check != idx);
  }

 private:
  da_type bc_;

};


template <typename OperationTag, typename ConstructionType, bool EdgeOrdering>
void PlainDaTrie<OperationTag, ConstructionType, EdgeOrdering>::Build(const KeysetHandler& keyset) {
  // A keys in keyset is required to be sorted and unique.
  using key_iterator = typename KeysetHandler::const_iterator;

  if constexpr (!EdgeOrdering) {

    size_t cnt_skip = 0;
    uint64_t time_fb = 0;
    auto dfs = [&](
        const auto dfs,
        const key_iterator begin,
        const key_iterator end,
        int depth,
        index_type da_index
    ) -> void {
      std::deque<uint8_t> children;
      assert(begin < end);
      auto keyit = begin;
      if (keyit->size() == depth) {
        children.push_back(kLeafChar);
        ++keyit;
      }

      std::vector<key_iterator> its;
      uint8_t pibot_char = kLeafChar;
      while (keyit < end) {
        uint8_t c = (*keyit)[depth];
        if (pibot_char < c) {
          children.push_back(c);
          its.push_back(keyit);
          pibot_char = c;
        }
        ++keyit;
      }
      its.push_back(end);

      assert(!children.empty());
      auto start_t = std::chrono::high_resolution_clock::now();
      auto base = bc_.FindBase(children, &cnt_skip);
      auto end_t = std::chrono::high_resolution_clock::now();
      time_fb += std::chrono::duration_cast<std::chrono::microseconds>(end_t-start_t).count();

      bc_[da_index].base = base;
      CheckExpand(Operate(base, children.back()));
      for (uint8_t c : children) {
        auto pos = Operate(base, c);
        SetEnabled(pos);
        bc_[pos].check = da_index;
      }

      if (children.front() == kLeafChar)
        children.pop_front();
      for (int i = 0; i < children.size(); i++) {
        dfs(dfs, its[i], its[i+1], depth+1, Operate(bc_[da_index].base, children[i]));
      }
    };
    const index_type root_index = 0;
    bc_.CheckExpand(root_index);
    bc_.SetEnabled(root_index);
    bc_[root_index].check = std::numeric_limits<index_type>::max();
    dfs(dfs, keyset.cbegin(), keyset.cend(), 0, root_index);

    std::cout << "\tCount roops: " << cnt_skip << std::endl;
    std::cout << "\tFindBase time: " << std::fixed << (double)time_fb/1000000 << " ￿s" << std::endl;

  } else {

    Build(RawTrie(keyset));

  }
}

template <typename OperationTag, typename ConstructionType, bool EdgeOrdering>
void PlainDaTrie<OperationTag, ConstructionType, EdgeOrdering>::Build(const RawTrie& trie) {
  // A keys in keyset is required to be sorted and unique.

  size_t cnt_skip = 0;
  uint64_t time_fb = 0;
  auto da_save_edges = [&](std::vector<uint8_t>& children, index_type da_index) {
    assert(!children.empty());
    auto start_t = std::chrono::high_resolution_clock::now();
    auto base = bc_.FindBase(children, &cnt_skip);
    auto end_t = std::chrono::high_resolution_clock::now();
    time_fb += std::chrono::duration_cast<std::chrono::microseconds>(end_t-start_t).count();

    bc_[da_index].base = base;
    bc_.CheckExpand(bc_.Operate(base, children.back()));
    for (uint8_t c : children) {
      auto pos = bc_.Operate(base, c);
      bc_.SetEnabled(pos);
      bc_[pos].check = da_index;
    }
  };

  if constexpr (!EdgeOrdering) {

    auto dfs = [&](
        const auto dfs,
        size_t trie_node,
        size_t da_index
    ) -> void {
      auto& edges = trie[trie_node];
      std::vector<uint8_t> children;
      children.reserve(edges.size());
      for (auto e : edges) {
        children.push_back(e.c);
      }

      da_save_edges(children, da_index);

      for (auto e : edges) {
        if (e.next == -1)
          continue;
        dfs(dfs, e.next, bc_.Operate(bc_[da_index].base, e.c));
      }
    };
    const index_type root_index = 0;
    bc_.CheckExpand(root_index);
    bc_.SetEnabled(root_index);
    bc_[root_index].check = std::numeric_limits<index_type>::max();
    dfs(dfs, 0, root_index);

  } else {

    std::vector<int> size(trie.size());
    auto set_trie_size = [&](const auto dfs, int s) -> int {
      int sz = 1;
      for (auto [c, t] : trie[s]) {
        if (t == -1)
          sz++;
        else
          sz += dfs(dfs, t);
      }
      size[s] = sz;
      return sz;
    };
    set_trie_size(set_trie_size, 0);

    auto dfs = [&](
        const auto dfs,
        int trie_node,
        index_type da_index
    ) -> void {
      auto& edges = trie[trie_node];
      std::vector<uint8_t> children;
      children.reserve(edges.size());
      for (auto e : edges)
        children.push_back(e.c);

      da_save_edges(children, da_index);

      std::deque<int> order(edges.size());
      std::iota(order.begin(), order.end(), 0);
      if (children[0] == kLeafChar)
        order.pop_front();
      std::sort(order.begin(), order.end(), [&](int l, int r) { return size[edges[l].next] > size[edges[r].next]; });
      for (auto i : order) {
        assert(edges[i].next != -1);
        dfs(dfs, trie[trie_node][i].next, bc_.Operate(bc_[da_index].base, children[i]));
      }
    };
    const index_type root_index = 0;
    bc_.CheckExpand(root_index);
    bc_.SetEnabled(root_index);
    bc_[root_index].check = std::numeric_limits<index_type>::max();
    dfs(dfs, 0, root_index);

  }

  std::cout << "\tCount roops: " << cnt_skip << std::endl;
  std::cout << "\tFindBase time: " << std::fixed << (double)time_fb/1000000 << " ￿s" << std::endl;
}

}

#endif //PLAIN_DA_TRIES__PLAIN_DA_HPP_
