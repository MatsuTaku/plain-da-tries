#ifndef PLAIN_DA_TRIES__KEYSET_HPP_
#define PLAIN_DA_TRIES__KEYSET_HPP_

#include <cstdint>
#include <vector>
#include <string_view>

namespace plain_da {

class KeysetHandler {
 public:
  using value_type = std::string_view;

  using iterator = std::vector<std::string_view>::iterator;
  using const_iterator = std::vector<std::string_view>::const_iterator;

 private:
  std::vector<uint8_t> storage_;
  std::vector<std::string_view> sv_list_;

  std::vector<std::pair<size_t, size_t>> pls_;

 public:
  KeysetHandler() = default;

  explicit KeysetHandler(std::istream& is) {
    for (std::string key; std::getline(is, key); )
      insert(key);
    update_list();
  }

  void insert(std::string_view key) {
    size_t front = storage_.size();
    storage_.resize(storage_.size() + key.length()+1);
    for (int i = 0; i < key.length(); i++)
      storage_[front + i] = key[i];
    storage_[front + key.length()] = '\0';
    pls_.emplace_back(front, key.length());
  }

  void update_list() {
    sv_list_.clear();
    sv_list_.reserve(pls_.size());
    for (auto [p, l] : pls_)
      sv_list_.emplace_back((const char*) storage_.data() + p, l);
  }

  size_t size() const { return sv_list_.size(); }

  std::string_view operator[](size_t i) const {
    return sv_list_[i];
  }

  const_iterator begin() const { return sv_list_.begin(); }
  const_iterator cbegin() const { return sv_list_.cbegin(); }

  const_iterator end() const { return sv_list_.end(); }
  const_iterator cend() const { return sv_list_.cend(); }

};


class RawTrie {
 public:
  static constexpr uint8_t kLeafChar = '\0';

  struct Edge {
    uint8_t c = kLeafChar;
    int next = -1;
  };

  using TableType = std::vector<std::vector<Edge>>;
  using const_iterator = TableType::const_iterator;

 private:
  TableType edge_table_;

 public:
  explicit RawTrie(const KeysetHandler& keyset) {
    using key_iterator = typename KeysetHandler::const_iterator;
    auto dfs = [&](
        const auto dfs,
        const key_iterator begin,
        const key_iterator end,
        int depth
    ) -> size_t {
      size_t cur_node = edge_table_.size();
      edge_table_.emplace_back();
      assert(begin < end);
      auto keyit = begin;
      if (keyit->size() == depth) {
        edge_table_[cur_node].push_back({kLeafChar, -1});
        ++keyit;
      }

      auto pit = keyit;
      uint8_t pibot_char = kLeafChar;
      while (keyit < end) {
        uint8_t c = (*keyit)[depth];
        if (pibot_char < c) {
          if (pit < keyit) {
            assert(!edge_table_[cur_node].empty());
            edge_table_[cur_node].back().next = dfs(dfs, pit, keyit, depth+1);
          }
          edge_table_[cur_node].push_back({c, -1});
          pit = keyit;
          pibot_char = c;
        }
        ++keyit;
      }
      if (pit < keyit) {
        assert(!edge_table_[cur_node].empty());
        edge_table_[cur_node].back().next = dfs(dfs, pit, keyit, depth+1);
      }
      return cur_node;
    };
    dfs(dfs, keyset.begin(), keyset.end(), 0);
  }

  size_t size() const { return edge_table_.size(); }

  const std::vector<Edge>& operator[](size_t idx) const {
    return edge_table_[idx];
  }

  const_iterator begin() const { return edge_table_.begin(); }
  const_iterator cbegin() const { return edge_table_.cbegin(); }

  const_iterator end() const { return edge_table_.end(); }
  const_iterator cend() const { return edge_table_.cend(); }

};

}

#endif //PLAIN_DA_TRIES__KEYSET_HPP_
