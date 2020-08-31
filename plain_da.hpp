#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <deque>
#include <limits>
#include <cassert>
#include <iostream>
#include <bitset>
#include <chrono>

#include <bo.hpp>

#include "bit_vector.hpp"
#include "keyset.hpp"

namespace plain_da {

template <int ConstructionType>
class PlainDa {
 public:
  using index_type = int32_t;

  static constexpr uint8_t kLeafChar = '\0';
  static constexpr index_type kInvalidIndex = -1;

 public:
  PlainDa() = default;

  explicit PlainDa(const KeysetHandler& keyset) {
    Build(keyset);
  }

  void Build(const KeysetHandler& keyset);

  explicit PlainDa(const RawTrie& trie) {
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
      auto nxt = bc_[idx].base + c;
      if (nxt >= bc_.size() or bc_[nxt].check != idx) {
        return false;
      }
      idx = nxt;
    }
    auto nxt = bc_[idx].base + kLeafChar;
    return !(nxt >= bc_.size() or bc_[nxt].check != idx);
  }

 private:
  struct DaUnit {
    index_type check = kInvalidIndex;
    index_type base = kInvalidIndex;
    index_type succ() const { return -check-1; }
    void set_succ(index_type nv) {
      check = -(nv+1);
    }
    index_type pred() const { return -base-1; }
    void set_pred(index_type nv) {
      base = -(nv+1);
    }
    bool Enabled() const { return check >= 0; }
  };

  std::vector<DaUnit> bc_;
  BitVector exists_bits_;
  index_type empty_head_ = kInvalidIndex;
  size_t cnt_skip_ = 0;
  uint64_t time_fb_ = 0;

  void SetDisabled(index_type pos);

  void SetEnabled(index_type pos);

  void CheckExpand(index_type pos);

  template <typename Container>
  index_type FindBase(const Container& children);

};


template <int ConstructionType>
void PlainDa<ConstructionType>::Build(const KeysetHandler& keyset) {
  // A keys in keyset is required to be sorted and unique.
  using key_iterator = typename KeysetHandler::const_iterator;
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
    auto base = FindBase(children);
    auto end_t = std::chrono::high_resolution_clock::now();
    time_fb_ += std::chrono::duration_cast<std::chrono::microseconds>(end_t-start_t).count();

    bc_[da_index].base = base;
    CheckExpand(base + children.back());
    for (uint8_t c : children) {
      auto pos = base + c;
      SetEnabled(pos);
      bc_[pos].check = da_index;
    }

    if (children.front() == kLeafChar)
      children.pop_front();
    for (int i = 0; i < children.size(); i++) {
      dfs(dfs, its[i], its[i+1], depth+1, base + children[i]);
    }
  };
  const index_type root_index = 0;
  CheckExpand(root_index);
  SetEnabled(root_index);
  bc_[root_index].check = std::numeric_limits<index_type>::max();
  dfs(dfs, keyset.cbegin(), keyset.cend(), 0, root_index);

  std::cout << "\tCount roops: " << cnt_skip_ << std::endl;
  std::cout << "\tFindBase time: " << std::fixed << (double)time_fb_/1000000 << " ￿s" << std::endl;
}

template <int ConstructionType>
void PlainDa<ConstructionType>::Build(const RawTrie& trie) {
  // A keys in keyset is required to be sorted and unique.
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

    assert(!children.empty());
    auto start_t = std::chrono::high_resolution_clock::now();
    auto base = FindBase(children);
    auto end_t = std::chrono::high_resolution_clock::now();
    time_fb_ += std::chrono::duration_cast<std::chrono::microseconds>(end_t-start_t).count();

    bc_[da_index].base = base;
    CheckExpand(base + children.back());
    for (uint8_t c : children) {
      auto pos = base + c;
      SetEnabled(pos);
      bc_[pos].check = da_index;
    }

    for (auto e : edges) {
      if (e.next == -1)
        continue;
      dfs(dfs, e.next, base + e.c);
    }
  };
  const index_type root_index = 0;
  CheckExpand(root_index);
  SetEnabled(root_index);
  bc_[root_index].check = std::numeric_limits<index_type>::max();
  dfs(dfs, 0, root_index);

  std::cout << "\tCount roops: " << cnt_skip_ << std::endl;
  std::cout << "\tFindBase time: " << std::fixed << (double)time_fb_/1000000 << " ￿s" << std::endl;
}

template <int ConstructionType>
void PlainDa<ConstructionType>::SetDisabled(index_type pos) {
  if (empty_head_ == kInvalidIndex) {
    empty_head_ = pos;
    bc_[pos].set_succ(pos);
    bc_[pos].set_pred(pos);
  } else {
    auto back_pos = bc_[empty_head_].pred();
    bc_[back_pos].set_succ(pos);
    bc_[empty_head_].set_pred(pos);
    bc_[pos].set_succ(empty_head_);
    bc_[pos].set_pred(back_pos);
  }
  if constexpr (ConstructionType != 0) {
    exists_bits_[pos] = false;
  }
}

template <int ConstructionType>
void PlainDa<ConstructionType>::SetEnabled(index_type pos) {
  assert(!bc_[pos].Enabled());
  auto succ_pos = bc_[pos].succ();
  if (pos == empty_head_) {
    empty_head_ = (succ_pos != pos) ? succ_pos : kInvalidIndex;
  }
  auto pred_pos = bc_[pos].pred();
  bc_[pred_pos].set_succ(succ_pos);
  bc_[succ_pos].set_pred(pred_pos);
  if constexpr (ConstructionType != 0) {
    exists_bits_[pos] = true;
  }
}

template <int ConstructionType>
void PlainDa<ConstructionType>::CheckExpand(index_type pos) {
  auto old_size = size();
  auto new_size = pos + 1;
  if (new_size <= old_size)
    return;
  bc_.resize(new_size);
  if constexpr (ConstructionType != 0) {
    exists_bits_.resize(new_size);
  }
  for (auto i = old_size; i < new_size; i++) {
    SetDisabled(i);
  }
}

template <int ConstructionType>
template <typename Container>
typename PlainDa<ConstructionType>::index_type
PlainDa<ConstructionType>::FindBase(const Container& children) {
  assert(!children.empty());
  uint8_t fstc = children[0];

  if (empty_head_ == kInvalidIndex)
    return size() - fstc;

  if (children.size() == 1)
    return empty_head_ - fstc;

  if constexpr (ConstructionType == 0) {

    index_type base_front = empty_head_ - fstc;
    auto base = base_front;
    while (base + fstc < size()) {
      bool ok = true;
      assert(!bc_[base + fstc].Enabled());
      for (int i = 1; i < children.size(); i++) {
        uint8_t c = children[i];
        ok &= base + c >= size() or !bc_[base + c].Enabled();
        if (!ok)
          break;
      }
      if (ok) {
        return base;
      }
      base = bc_[base + fstc].succ() - fstc;
      if (base == base_front)
        break;
      cnt_skip_++;
    }
    return size() - fstc;

  } else {

    for (int offset = empty_head_-fstc; offset+fstc < size(); ) {
      uint64_t bits = 0ull;
      for (uint8_t c : children) {
        bits |= exists_bits_.bits64(offset + c);
        if (~bits == 0ull)
          break;
      }
      bits = ~bits;
      if (bits != 0ull) {
        return offset + bo::ctz_u64(bits);
      }

      if constexpr (ConstructionType == 1) {

        offset += 64;

      } else if constexpr (ConstructionType == 2) {

        auto window_front = offset + fstc;
        uint64_t word_with_fstc = ~exists_bits_.bits64(window_front);
        assert(word_with_fstc != 0ull);
        auto clz = bo::clz_u64(word_with_fstc);
        auto window_empty_tail = window_front + 63 - clz;
        if (window_empty_tail >= size())
          break;
        assert(!bc_[window_empty_tail].Enabled());
        auto next_empty_pos = bc_[window_empty_tail].succ();
        assert(!bc_[next_empty_pos].Enabled());
        if (next_empty_pos == empty_head_)
          break;
        assert(next_empty_pos - window_front >= 64);
        offset = next_empty_pos - fstc;

      }
      cnt_skip_++;
    }
    return size() - fstc;

  }
}

}