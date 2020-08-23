#include <cstdint>
#include <string>
#include <vector>
#include <deque>

namespace plain_da {

template <int ConstructionType>
class PlainDa {
 public:
  using index_type = int32_t;

  static constexpr uint8_t kLeafChar = '\0';

 public:
  explicit PlainDa(const std::vector<std::string>& keyset);

  bool contains(const std::string& key) const {
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
    static constexpr index_type kInitialValue = -1;
    index_type check = kInitialValue;
    index_type base = kInitialValue;
  };

  std::vector<DaUnit> bc_;
  std::vector<bool> exists_bits_;

  size_t size() const { return bc_.size(); }

  void CheckExpand(size_t pos) {
    if (pos < size())
      return;
    auto new_size = pos + 1;
    bc_.resize(new_size);
    exists_bits_.resize(new_size);
  }

  template <typename Container>
  index_type FindBase(const Container& children) const;

};


template <int ConstructionType>
PlainDa<ConstructionType>::PlainDa(const std::vector<std::string>& keyset) {
  // A keys in keyset is required to be sorted and unique.
  using key_iterator = std::vector<std::string>::const_iterator;
  auto dfs = [&](
      const auto dfs,
      const key_iterator begin,
      const key_iterator end,
      int depth,
      index_type da_index
      ) -> void {
    std::deque<uint8_t> children;
    auto keyit = begin;
    if (keyit->size() == depth) {
      children.push_back(kLeafChar);
      ++keyit;
      if (keyit == end)
        return;
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

    auto base = FindBase(children);
    bc_[da_index].base = base;
    CheckExpand(base + children.back());
    for (uint8_t c : children) {
      auto pos = base + c;
      bc_[pos].check = da_index;
      exists_bits_[pos] = true;
    }

    if (children.front() == kLeafChar)
      children.pop_front();
    for (int i = 0; i < its.size()-1; i++) {
      dfs(dfs, its[i], its[i+1], depth+1, base + children[i]);
    }
  };
  const index_type root_index = 0;
  CheckExpand(root_index);
  bc_[root_index].check = std::numeric_limits<index_type>::max();
  exists_bits_[root_index] = true;
  dfs(dfs, keyset.cbegin(), keyset.cend(), 0, root_index);
}

template <int ConstructionType>
template <typename Container>
typename PlainDa<ConstructionType>::index_type
PlainDa<ConstructionType>::FindBase(const Container& children) const {
  if constexpr        (ConstructionType == 0) {

  } else if constexpr (ConstructionType == 1) {

  } else if constexpr (ConstructionType == 2) {

  }
  // TODO: Needs to implementations
  return 0;
}

}