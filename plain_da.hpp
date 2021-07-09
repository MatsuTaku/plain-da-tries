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
#include <stdexcept>

#include "double_array_base.hpp"
#include "tail.hpp"
#include "keyset.hpp"

namespace plain_da {

template <typename DaType, bool EdgeOrdering>
class PlainDaTrie {
 public:
  using da_type = DaType;

 private:
  da_type bc_;

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

};

template <typename DaType, bool EdgeOrdering>
void PlainDaTrie<DaType, EdgeOrdering>::Build(const KeysetHandler& keyset) {
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
        assert(!bc_[pos].Enabled());
        if (bc_[pos].Enabled()) {
          throw std::logic_error("FindBase is not implemented correctly!");
        }
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

template <typename DaType, bool EdgeOrdering>
void PlainDaTrie<DaType, EdgeOrdering>::Build(const RawTrie& trie) {
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
      assert(!bc_[pos].Enabled());
      if (bc_[pos].Enabled()) {
        throw std::logic_error("FindBase is not implemented correctly!");
      }
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


template <typename DaType, bool EdgeOrdering>
class PlainDaMpTrie {
 public:
  using da_type = DaType;

 private:
  da_type bc_;
  Tail tail_;

 public:
  PlainDaMpTrie() = default;

  explicit PlainDaMpTrie(const KeysetHandler& keyset) {
    Build(keyset);
  }
  void Build(const KeysetHandler& keyset);

  explicit PlainDaMpTrie(const RawTrie& trie) {
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
    auto it = key.begin();
    for (; it != key.end(); ++it) {
      if (!bc_[idx].HasBase())
        break;
      auto nxt = bc_.Operate(bc_[idx].base(), *it);
      if (nxt >= bc_.size() or bc_[nxt].check() != idx) {
        return false;
      }
      idx = nxt;
    }
    if (bc_[idx].HasBase()) { // Check leaf transition
      if (it != key.end())
        return false;
      auto nxt = bc_.Operate(bc_[idx].base(), kLeafChar);
      return nxt < bc_.size() and bc_[nxt].check() == idx;
    } else { // Compare on a TAIL
      size_t tail_i = bc_[idx].tail_i();
      for (; it != key.end(); ++it, ++tail_i) {
        if (tail_i < tail_.size() and *it != tail_[tail_i])
          return false;
      }
      return tail_i < tail_.size() and tail_[tail_i] == (char) kLeafChar;
    }
  }

};

template <typename DaType, bool EdgeOrdering>
void PlainDaMpTrie<DaType, EdgeOrdering>::Build(const KeysetHandler& keyset) {
  // A keys in keyset is required to be sorted and distinct for each keys.
  using key_iterator = typename KeysetHandler::const_iterator;

  if constexpr (!EdgeOrdering) {

    TailConstructor tail_constr;

    size_t cnt_skip = 0;
    uint64_t time_fb = 0;
    auto dfs = [&](
        const auto dfs,
        const key_iterator begin,
        const key_iterator end,
        int depth,
        index_type da_index
    ) -> void {
      assert(begin < end);

      if (std::next(begin) == end) { // Store on TAIL
        auto idx = tail_constr.push(std::string(begin->substr(depth)));
        bc_[da_index].set_tail_i(idx);
        return;
      }

      std::deque<uint8_t> children;
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
        assert(!bc_[pos].Enabled());
        if (bc_[pos].Enabled()) {
          throw std::logic_error("FindBase is not implemented correctly!");
        }
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

    tail_constr.Construct();
    for (size_t i = 0; i < bc_.size(); i++) {
      if (!bc_[i].Enabled() or bc_[i].HasBase())
        continue;
      bc_[i].set_tail_i(tail_constr.map_to(bc_[i].tail_i()));
    }
    tail_ = Tail(std::move(tail_constr));

    std::cout << "\tCount roops: " << cnt_skip << std::endl;
    std::cout << "\tFindBase time: " << std::fixed << (double)time_fb/1000000 << " ￿s" << std::endl;

  } else {

    Build(RawTrie(keyset));

  }
}

template <typename DaType, bool EdgeOrdering>
void PlainDaMpTrie<DaType, EdgeOrdering>::Build(const RawTrie& trie) {
  // A keys in keyset is required to be sorted and unique.

  size_t cnt_skip = 0;
  uint64_t time_fb = 0;

  std::vector<bool> to_leaf(trie.size());
  auto set_to_leaf = [&](auto f, size_t trie_node) {
    if (trie_node == -1) return;
    auto& edges = trie[trie_node];
    for (auto &e : edges) {
      f(f, e.next);
    }
    if (edges.size() == 1) {
      if (edges[0].c == kLeafChar) {
        to_leaf[trie_node] = true;
      } else {
        to_leaf[trie_node] = to_leaf[edges[0].next];
      }
    }
  };
  set_to_leaf(set_to_leaf, 0);

  auto get_suffix_rev = [f = [&trie](auto f, int trie_node, std::string& suf) -> void {
    auto& edges = trie[trie_node];
    assert(edges.size() == 1);
    if (edges[0].c != kLeafChar) {
      suf += edges[0].c;
      f(f, edges[0].next, suf);
    }
  }](int trie_node) {
    std::string suf = "";
    f(f, trie_node, suf);
    return suf;
  };

  TailConstructor tail_constr;
  auto da_save_edges = [&](const std::vector<uint8_t>& children, index_type da_index) {
    assert(!children.empty());
    auto start_t = std::chrono::high_resolution_clock::now();
    auto base = bc_.FindBase(children, &cnt_skip);
    auto end_t = std::chrono::high_resolution_clock::now();
    time_fb += std::chrono::duration_cast<std::chrono::microseconds>(end_t-start_t).count();

    bc_[da_index].set_base(base);
    bc_.CheckExpand(bc_.Operate(base, children.back()));
    for (uint8_t c : children) {
      auto pos = bc_.Operate(base, c);
      assert(!bc_[pos].Enabled());
      if (bc_[pos].Enabled()) {
        throw std::logic_error("FindBase is not implemented correctly!");
      }
      bc_.SetEnabled(pos);
      bc_[pos].set_check(da_index);
    }
  };

  std::vector<int> subtree_size;
  if constexpr (EdgeOrdering) {
    subtree_size.resize(trie.size());
    auto set_trie_size = [&](const auto dfs, int s) -> void {
      int& sz = subtree_size[s] = 1;
      for (auto [c, t] : trie[s]) {
        if (t == -1)
          sz++;
        else {
          dfs(dfs, t);
          sz += subtree_size[t];
        }
      }
    };
    set_trie_size(set_trie_size, 0);
  }

  auto dfs = [&](
      const auto dfs,
      int trie_node,
      index_type da_index
  ) -> void {
    if (to_leaf[trie_node]) { // Store on the TAIL
      auto suffix = get_suffix_rev(trie_node);
      auto idx = tail_constr.push(suffix);
      bc_[da_index].set_tail_i(idx);
      return;
    }

    auto& edges = trie[trie_node];
    std::vector<uint8_t> children;
    children.reserve(edges.size());
    for (auto e : edges)
      children.push_back(e.c);

    da_save_edges(children, da_index);

    std::deque<int> order(edges.size());
    std::iota(order.begin(), order.end(), 0);
    if (children[0] == kLeafChar) // (edges[0].next == -1)
      order.pop_front();
    if constexpr (EdgeOrdering) {
      std::sort(order.begin(), order.end(), [&](int l, int r) {
        return subtree_size[edges[l].next] > subtree_size[edges[r].next];
      });
    }
    for (auto i : order) {
      assert(edges[i].next != -1);
      dfs(dfs, trie[trie_node][i].next, bc_.Operate(bc_[da_index].base(), children[i]));
    }
  };
  const index_type root_index = 0;
  bc_.CheckExpand(root_index);
  bc_.SetEnabled(root_index);
  bc_[root_index].set_check(std::numeric_limits<index_type>::max());
  dfs(dfs, 0, root_index);

  tail_constr.Construct();
  for (size_t i = 0; i < bc_.size(); i++) {
    if (!bc_[i].Enabled() or bc_[i].HasBase())
      continue;
    auto c = bc_.RestoreLabel(bc_[bc_[i].check()].base(), i);
    if (c == kLeafChar)
      continue;
    auto tail_i = tail_constr.map_to(bc_[i].tail_i());
    assert(tail_i > 0);
    bc_[i].set_tail_i(tail_i);
  }
  tail_ = Tail(std::move(tail_constr));

  std::cout << "\tCount roops: " << cnt_skip << std::endl;
  std::cout << "\tFindBase time: " << std::fixed << (double)time_fb/1000000 << " ￿s" << std::endl;
}

}

#endif //PLAIN_DA_TRIES__PLAIN_DA_HPP_
