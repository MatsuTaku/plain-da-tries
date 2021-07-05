#ifndef PLAIN_DA_TRIES__BIT_VECTOR_HPP_
#define PLAIN_DA_TRIES__BIT_VECTOR_HPP_

#include <cstdint>
#include <vector>

namespace plain_da {

class BitVector : private std::vector<uint64_t> {
  using _base = std::vector<uint64_t>;

 private:
  size_t size_;

 public:
  BitVector() : size_(0) {}
  explicit BitVector(size_t size) : _base(size > 0 ? (size-1)/64+1 : 0), size_(size) {}

  size_t size() const {
    return size_;
  }

  void resize(size_t new_size) {
    _base::resize(new_size > 0 ? (new_size-1)/64+1 : 0);
    size_ = new_size;
  }

  const uint64_t* data() const { return _base::data(); }
  uint64_t* data() { return _base::data(); }

  uint64_t word(size_t wi) const {
    return wi < _base::size() ? _base::operator[](wi) : 0ull;
  }

  uint64_t bits64(size_t offset) const {
    auto inset = offset % 64;
    auto block = offset / 64;
    if (inset == 0) {
      return word(block);
    } else {
      return (word(block) >> inset) | (word(block + 1) << (64 - inset));
    }
  }

  class reference {
   public:
    using pointer = uint64_t*;
   private:
    pointer ptr_;
    uint64_t mask_;

    friend class BitVector;
    reference(pointer ptr, uint64_t mask) : ptr_(ptr), mask_(mask) {}

   public:
    operator bool() const {
      return (*ptr_ & mask_) != 0;
    }

    reference operator=(bool bit) {
      if (bit) {
        *ptr_ |= mask_;
      } else {
        *ptr_ &= ~mask_;
      }
      return *this;
    }
  };

  class const_reference {
   public:
    using pointer = const uint64_t*;
   private:
    pointer ptr_;
    uint64_t mask_;

    friend class BitVector;
    const_reference(pointer ptr, uint64_t mask) : ptr_(ptr), mask_(mask) {}

   public:
    operator bool() const {
      return (*ptr_ & mask_) != 0;
    }
  };

  reference operator[](size_t pos) {
    return reference(_base::data() + pos/64, 1ull<<(pos%64));
  }

  const_reference operator[](size_t pos) const {
    return const_reference(_base::data() + pos/64, 1ull<<(pos%64));
  }

 private:
  template<bool IsConst>
  class _iterator {
   public:
    using reference = std::conditional_t<!IsConst, BitVector::reference, BitVector::const_reference>;
    using pointer = std::conditional_t<!IsConst, uint64_t*, const uint64_t*>;
    using value_type = reference;
    using difference_type = long long;
    using iterator_category = std::random_access_iterator_tag;
   private:
    pointer ptr_;
    uint8_t ctz_;

    friend class BitVector;
    _iterator(pointer ptr, unsigned ctz) : ptr_(ptr), ctz_(ctz) {}

   public:
    reference operator*() const {
      return reference(ptr_, 1ull << ctz_);
    }

    _iterator& operator++() {
      if (ctz_ < 63)
        ++ctz_;
      else {
        ++ptr_;
        ctz_ = 0;
      }
      return *this;
    }
    _iterator operator++(int) {
      _iterator ret = *this;
      ++*this;
      return ret;
    }
    _iterator& operator--() {
      if (ctz_ > 0)
        --ctz_;
      else {
        --ptr_;
        ctz_ = 63;
      }
    }
    _iterator operator--(int) {
      _iterator ret = *this;
      --*this;
      return ret;
    }
    _iterator operator+(long long shifts) const {
      long long i = ctz_ + shifts;
      if (i >= 0) {
        return _iterator(ptr_ + i / 64, i % 64);
      } else {
        return _iterator(ptr_ - (-i-1) / 64 + 1, (i % 64 + 64) % 64);
      }
    }
    friend _iterator operator+(difference_type shifts, _iterator it) {
      return it + shifts;
    }
    _iterator& operator+=(difference_type shifts) {
      return *this = *this + shifts;
    }
    _iterator operator-(difference_type shifts) const {
      return operator+(-shifts);
    }
    _iterator& operator-=(difference_type shifts) {
      return *this = *this - shifts;
    }
    difference_type operator-(_iterator rhs) const {
      return difference_type(ptr_ - rhs.ptr_) * 64 + ((difference_type) ctz_ - rhs.ctz_);
    }
    reference operator[](size_t i) const {
      return *(*this + i);
    }
    bool operator<(_iterator rhs) const {
      return ptr_ != rhs.ptr_ ? ptr_ < rhs.ptr_ : ctz_ < rhs.ctz_;
    }
    bool operator>(_iterator rhs) const {
      return rhs < *this;
    }
    bool operator==(_iterator rhs) const {
      return ptr_ == rhs.ptr_ and ctz_ == rhs.ctz_;
    }
    bool operator<=(_iterator rhs) const {
      return ptr_ != rhs.ptr_ ? ptr_ < rhs.ptr_ : ctz_ <= rhs.ctz_;
    }
    bool operator>=(_iterator rhs) const {
      return rhs <= *this;
    }
  };

 public:
  using iterator = _iterator<false>;
  using const_iterator = _iterator<true>;

  iterator begin() {
    return iterator(_base::data(), 0);
  }
  const_iterator cbegin() const {
    return const_iterator(_base::data(), 0);
  }
  const_iterator begin() const {
    return cbegin();
  }
  iterator end() {
    return iterator(_base::data() + (size()-1) / 64, (size() - 1) % 64);
  }
  const_iterator cend() const {
    return const_iterator(_base::data() + (size()-1) / 64, (size() - 1) % 64);
  }
  const_iterator end() const {
    return cend();
  }

};

}

#endif //PLAIN_DA_TRIES__BIT_VECTOR_HPP_
