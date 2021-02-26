#ifndef PLAIN_DA_TRIES__TAIL_HPP_
#define PLAIN_DA_TRIES__TAIL_HPP_

#include <vector>
#include <string>
#include <string_view>
#include <queue>
#include <cassert>

namespace plain_da {

class TailConstructor {
 public:
  std::vector<std::pair<std::string, size_t>> key_vec_;
  std::vector<size_t> index_;
  std::vector<char> arr_;

  friend class Tail;

 public:
  size_t push(const std::string& key) {
    auto id = key_vec_.size() + 1;
    key_vec_.emplace_back(key, id);
    return id;
  }

  void Construct() {
    if (key_vec_.empty())
      return;
    arr_.resize(1, kLeafChar);
    size_t n = key_vec_.size();
    std::sort(key_vec_.begin(), key_vec_.end(), [](auto& l, auto& r) {
      auto& lkey = l.first;
      auto& rkey = r.first;
      return std::lexicographical_compare(lkey.rbegin(), lkey.rend(), rkey.rbegin(), rkey.rend());
    });
    index_.resize(n+1, -1);
    auto it = key_vec_.begin();
    std::queue<std::pair<size_t, size_t>> idqueue;
    idqueue.emplace(it->second, it->first.length());
    std::string* prev = &it->first;
    ++it;
    auto construct = [&]() {
      for (uint8_t c : *prev)
        arr_.push_back(c);
      arr_.push_back(kLeafChar);
      while (!idqueue.empty()) {
        auto [id, len] = idqueue.front(); idqueue.pop();
        assert(id > 0);
        assert(len <= prev->length());
        index_[id] = arr_.size() - 1 - len;
        assert(index_[id] > 0);
        if (index_[id] >= 1 << 31) {
          throw "Too large tail length for embedded 31bit pointer.";
        }
      }
    };
    for (; it != key_vec_.end(); ++it) {
      auto& [key, id] = *it;
      bool mergeable = prev->length() <= key.length();
      if (mergeable) {
        auto rit = prev->rbegin();
        auto kit = key.rbegin();
        for (; mergeable and rit != prev->rend(); ++rit, ++kit) {
          mergeable &= *rit == *kit;
        }
      }
      if (!mergeable) {
        construct();
      }
      prev = &it->first;
      idqueue.emplace(id, key.length());
    }
    construct();
    arr_.shrink_to_fit();
  }

  size_t map_to(size_t id) const {
    return index_[id];
  }
};

class Tail {
 private:
  std::vector<char> arr_;

 public:
  Tail() = default;
  explicit Tail(TailConstructor&& constructor) : arr_(std::move(constructor.arr_)) {}

  size_t size() const { return arr_.size(); }

  char operator[](size_t i) const { return arr_[i]; }

  std::string_view label(size_t i) const {
    return std::string_view(arr_.data() + i);
  }
};

}

#endif //PLAIN_DA_TRIES__TAIL_HPP_
