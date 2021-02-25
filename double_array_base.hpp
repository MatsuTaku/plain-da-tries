#ifndef PLAIN_DA_TRIES__DOUBLE_ARRAY_BASE_HPP_
#define PLAIN_DA_TRIES__DOUBLE_ARRAY_BASE_HPP_

#include <cstdint>
#include <vector>
#include <array>

#include <bo.hpp>

#include "bit_vector.hpp"

namespace plain_da {

using index_type = int32_t;

constexpr uint8_t kLeafChar = '\0';
constexpr index_type kInvalidIndex = -1;


struct da_plus_operation_tag {};
struct da_xor_operation_tag {};

template<typename OperationTag>
struct DaOperation{};

template<>
struct DaOperation<da_plus_operation_tag> {
  index_type operator()(index_type base, uint8_t c) const {
    return base + c;
  }
  index_type inv(index_type base, uint8_t c) const {
    return base - c;
  }
};

template<>
struct DaOperation<da_xor_operation_tag> {
  index_type operator()(index_type base, uint8_t c) const {
    return base ^ c;
  }
  index_type inv(index_type base, uint8_t c) const {
    return base ^ c;
  }
};


struct da_construction_type_ELM {};
struct da_construction_type_WW {};
struct da_construction_type_WW_ELM {};


template <typename OperationTag, typename ConstructionType>
class DoubleArrayBase {
 public:
  using op_type = DaOperation<OperationTag>;

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

 private:
  op_type operation_;
  std::vector<DaUnit> bc_;
  BitVector exists_bits_;
  index_type empty_head_ = kInvalidIndex;

 public:
  size_t size() const { return bc_.size(); }
  
  index_type Operate(index_type base, uint8_t c) const { return operation_(base, c); }

  const DaUnit& operator[](size_t i) const {
    return bc_[i];
  }
  DaUnit& operator[](size_t i) {
    return bc_[i];
  }

  void SetDisabled(index_type pos);

  void SetEnabled(index_type pos);

  void CheckExpand(index_type pos);

  template <typename Container>
  index_type FindBase(const Container& children, size_t* counter) const;

};

template <typename OperationTag, typename ConstructionType>
void DoubleArrayBase<OperationTag, ConstructionType>::SetDisabled(index_type pos) {
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
  if constexpr (!std::is_same_v<ConstructionType, da_construction_type_ELM>) {
    exists_bits_[pos] = false;
  }
}

template <typename OperationTag, typename ConstructionType>
void DoubleArrayBase<OperationTag, ConstructionType>::SetEnabled(index_type pos) {
  assert(!bc_[pos].Enabled());
  auto succ_pos = bc_[pos].succ();
  if (pos == empty_head_) {
    empty_head_ = (succ_pos != pos) ? succ_pos : kInvalidIndex;
  }
  auto pred_pos = bc_[pos].pred();
  bc_[pred_pos].set_succ(succ_pos);
  bc_[succ_pos].set_pred(pred_pos);
  if constexpr (!std::is_same_v<ConstructionType, da_construction_type_ELM>) {
    exists_bits_[pos] = true;
  }
}

template <typename OperationTag, typename ConstructionType>
void DoubleArrayBase<OperationTag, ConstructionType>::CheckExpand(index_type pos) {
  auto old_size = size();
  auto new_size = ((pos/256)+1)*256;
  if (new_size <= old_size)
    return;
  bc_.resize(new_size);
  if constexpr (!std::is_same_v<ConstructionType, da_construction_type_ELM>) {
    exists_bits_.resize(new_size);
  }
  for (auto i = old_size; i < new_size; i++) {
    SetDisabled(i);
  }
}

template <typename OperationTag, typename ConstructionType>
template <typename Container>
index_type DoubleArrayBase<OperationTag, ConstructionType>::FindBase(const Container& children, size_t* counter) const {

  assert(!children.empty());
  uint8_t fstc = children[0];

  if (empty_head_ == kInvalidIndex)
    return operation_.inv(size(), fstc);

  if (children.size() == 1)
    return operation_.inv(empty_head_, fstc);

  if constexpr (std::is_same_v<ConstructionType, da_construction_type_ELM>) {

    index_type base_front = operation_.inv(empty_head_, fstc);
    auto base = base_front;
    while (operation_(base, fstc) < size()) {
      bool ok = true;
      assert(!bc_[operation_(base, fstc)].Enabled());
      for (int i = 1; i < children.size(); i++) {
        uint8_t c = children[i];
        ok &= operation_(base, c) >= size() or !bc_[operation_(base, c)].Enabled();
        if (!ok)
          break;
      }
      if (ok) {
        return base;
      }
      base = InvOperate(bc_[operation_(base, fstc)].succ(), fstc);
      if (base == base_front)
        break;
      if (counter) (*counter)++;
    }
    return InvOperate(size(), fstc);

  } else {

    if constexpr (std::is_same_v<OperationTag, da_plus_operation_tag>) {

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

        if constexpr (std::is_same_v<ConstructionType, da_construction_type_WW>) {

          offset += 64;

        } else if constexpr (std::is_same_v<ConstructionType, da_construction_type_WW_ELM>) {

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
        if (counter) (*counter)++;
      }
      return size() - fstc;

    } else if constexpr (std::is_same_v<OperationTag, da_xor_operation_tag>) {

      size_t b = empty_head_/256;
      size_t bend = size()/256;
      for (; b < bend; ++b) {
        std::array<uint64_t, 4> bits{};
        for (uint8_t c : children) {
          std::array<uint64_t, 4> exists_word;
          std::memcpy(exists_word.data(), exists_bits_.data()+(b*4), sizeof(uint64_t)*4);
          if (c & (1<<0))
            for (int i = 0; i < 4; i++)
              exists_word[i] |= ((exists_word[i] >> 1) & 0x5555555555555555ull) | ((exists_word[i] & 0x5555555555555555ull) << 1);
          if (c & (1<<1))
            for (int i = 0; i < 4; i++)
              exists_word[i] |= ((exists_word[i] >> 2) & 0x3333333333333333ull) | ((exists_word[i] & 0x3333333333333333ull) << 2);
          if (c & (1<<2))
            for (int i = 0; i < 4; i++)
              exists_word[i] |= ((exists_word[i] >> 4) & 0x0F0F0F0F0F0F0F0Full) | ((exists_word[i] & 0x0F0F0F0F0F0F0F0Full) << 4);
          if (c & (1<<3))
            for (int i = 0; i < 4; i++)
              exists_word[i] |= ((exists_word[i] >> 8) & 0x00FF00FF00FF00FFull) | ((exists_word[i] & 0x00FF00FF00FF00FFull) << 8);
          if (c & (1<<4))
            for (int i = 0; i < 4; i++)
              exists_word[i] |= ((exists_word[i] >> 16) & 0x0000FFFF0000FFFFull) | ((exists_word[i] & 0x0000FFFF0000FFFFull) << 16);
          if (c & (1<<5))
            for (int i = 0; i < 4; i++)
              exists_word[i] |= (exists_word[i] >> 32) | (exists_word[i] << 32);
          if (c & (1<<6)) {
            std::swap(exists_word[0], exists_word[1]);
            std::swap(exists_word[2], exists_word[3]);
          }
          if (c & (1<<7)) {
            std::swap(exists_word[0], exists_word[2]);
            std::swap(exists_word[1], exists_word[3]);
          }
          for (int i = 0; i < 4; i++)
            bits[i] |= exists_word[i];
        }
        for (int i = 0; i < 4; i++) {
          if (~bits[i] != 0ull) {
            auto inset = bo::ctz_u64(~bits[i]);
            return b*256 + i*64 + inset;
          }
        }
        if (counter) (*counter)++;
      }
      return size();

    }

  }
}

}

#endif //PLAIN_DA_TRIES__DOUBLE_ARRAY_BASE_HPP_