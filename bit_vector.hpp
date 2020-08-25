#ifndef PLAIN_DA_TRIES__BIT_VECTOR_HPP_
#define PLAIN_DA_TRIES__BIT_VECTOR_HPP_

#include <cstdint>
#include <vector>

namespace plain_da {

class BitVector : std::vector<uint64_t> {
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

  uint64_t word(size_t wi) const {
    return (wi < (size()-1)/64+1) ? *(_base::data() + wi) : 0ull;
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

  using pointer = uint64_t*;
  using const_pointer = const uint64_t*;

  class reference {
   private:
    pointer ptr_;
    uint64_t mask_;

    friend class BitVector;
    reference(pointer ptr, unsigned mask) : ptr_(ptr), mask_(mask) {}

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
   private:
    const_pointer ptr_;
    uint64_t mask_;

    friend class BitVector;
    const_reference(const_pointer ptr, unsigned mask) : ptr_(ptr), mask_(mask) {}

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

};

}

#endif //PLAIN_DA_TRIES__BIT_VECTOR_HPP_
