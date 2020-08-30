#ifndef PLAIN_DA_TRIES__KEYSET_HANDLER_HPP_
#define PLAIN_DA_TRIES__KEYSET_HANDLER_HPP_

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

 public:
  explicit KeysetHandler(std::istream& is) {
    std::vector<std::pair<size_t, size_t>> pls;
    for (std::string key; std::getline(is, key); ) {
      auto front = storage_.size();
      storage_.resize(storage_.size() + key.length() + 1);
      for (int i = 0; i < key.length(); i++)
        storage_[front + i] = key[i];
      storage_[front + key.size()] = '\0';
      pls.emplace_back(front, key.length());
    }
    sv_list_.reserve(pls.size());
    for (auto [p, l] : pls)
      sv_list_.emplace_back((const char*) storage_.data() + p, l);
  }

  std::string_view operator[](size_t i) const {
    return sv_list_[i];
  }

  size_t size() const { return sv_list_.size(); }

  iterator begin() { return sv_list_.begin(); }
  const_iterator begin() const { return sv_list_.begin(); }
  const_iterator cbegin() const { return sv_list_.cbegin(); }

  iterator end() { return sv_list_.end(); }
  const_iterator end() const { return sv_list_.end(); }
  const_iterator cend() const { return sv_list_.cend(); }

};


template <typename T>
struct KeysetTraits {
  using value_type = typename T::value_type;
  using iterator = typename T::iterator;
  using const_iterator = typename T::const_iterator;
};

}

#endif //PLAIN_DA_TRIES__KEYSET_HANDLER_HPP_
