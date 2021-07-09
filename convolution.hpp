#ifndef PLAIN_DA_TRIES__CONVOLUTION_HPP_
#define PLAIN_DA_TRIES__CONVOLUTION_HPP_

#include <cstdint>
#include <vector>
#include <stdexcept>

#include <bo.hpp>

namespace plain_da::convolution {

// Index SUM(+) convolution

using uint = uint32_t;

template<uint MOD>
class Modulo {
 private:
  uint v_;
 public:
  constexpr Modulo() : v_(0) {}
  template<typename T>
  constexpr Modulo(T v) : v_(v >= 0 ? v % (T) MOD : v % (T) MOD + (T) MOD) {}

  constexpr uint val() const {
    return v_;
  }
  constexpr bool operator==(Modulo x) const {
    return v_ == x.v_;
  }
  constexpr bool operator!=(Modulo x) const {
    return v_ != x.v_;
  }

  Modulo operator+() const {
    return *this;
  }
  Modulo operator-() const {
    return {MOD - v_};
  }
  constexpr Modulo operator+(Modulo x) const {
    return {v_ + x.v_};
  }
  constexpr Modulo operator-(Modulo x) const {
    return *this + -x;
  }
  constexpr Modulo operator*(Modulo x) const {
    return {(unsigned long long) v_ * x.v_};
  }
  friend constexpr Modulo pow(Modulo x, uint p) {
    Modulo t = 1;
    Modulo u = x;
    while (p > 0) {
      if (p & 1) {
        t *= u;
      }
      u *= u;
      p >>= 1;
    }
    return t;
  }
  constexpr Modulo inv() const {
    return pow(*this, MOD-2);
  }
  constexpr Modulo operator/(Modulo x) const {
    return *this * x.inv();
  }

  constexpr Modulo& operator+=(Modulo x) {
    return *this = *this + x;
  }
  constexpr Modulo& operator-=(Modulo x) {
    return *this = *this - x;
  }
  constexpr Modulo& operator*=(Modulo x) {
    return *this = *this * x;
  }
  constexpr Modulo& operator/=(Modulo x) {
    return *this = *this / x;
  }
};

template<typename T>
void bit_reverse(T f[], size_t n) {
  for (size_t i = 0, j = 1; j < n-1; j++) {
    for (size_t k = n >> 1; k > (i ^= k); k >>= 1) {}
    if (i < j) std::swap(f[i], f[j]);
  }
}

constexpr uint kModNTT = 998244353;
constexpr int kDivLim = 23;
using ModuloNTT = Modulo<kModNTT>;
constexpr ModuloNTT kPrimitiveRoot = 3;

// Number Theoretic Transform
template<bool INV>
void _ntt(ModuloNTT f[], size_t n) {
  if (n == 1)
    return;
  if (n > 1<<23) {
    throw std::logic_error("Length of input array of NTT is too long.");
  }
  static bool initialized = false;
  static ModuloNTT es[kDivLim+1], ies[kDivLim+1];
  if (!initialized) {
    initialized = true;
    es[kDivLim] = pow(kPrimitiveRoot, (kModNTT-1)>>kDivLim);
    for (int i = kDivLim-1; i >= 0; i--) {
      es[i] = es[i+1] * es[i+1];
    }
    ies[kDivLim] = es[kDivLim].inv();
    for (int i = kDivLim-1; i >= 0; i--) {
      ies[i] = ies[i+1] * ies[i+1];
    }
  }

  bit_reverse(f, n);
  for (int s = 1; 1 << s <= n; s++) {
    const size_t m = 1 << s;
    const auto wm = !INV ? es[s] : ies[s];
    for (size_t k = 0; k < n; k += m) {
      ModuloNTT w = 1;
      for (size_t j = 0; j < m/2; j++) {
        auto a = f[k + j];
        auto b = f[k + j + m/2] * w;
        f[k + j]       = a + b;
        f[k + j + m/2] = a - b;
        w *= wm;
      }
    }
  }

  if constexpr (INV) {
    auto invn = ModuloNTT(n).inv();
    for (size_t i = 0; i < n; i++)
      f[i] *= invn;
  }
}

void ntt(ModuloNTT f[], size_t n) {
  _ntt<0>(f, n);
}
void intt(ModuloNTT f[], size_t n) {
  _ntt<1>(f, n);
}

void index_sum_convolution_for_xcheck(ModuloNTT f[], ModuloNTT Tg[], size_t n) {
  assert(bo::popcnt_u64(n) == 1);
  ntt(f, n);
  for (size_t i = 0; i < n; i++) {
    f[i] *= Tg[i];
  }
  intt(f, n);
}


// Index XOR(^) convolution

// Fast Walsh-Hadamard Transform for XOR-Convolution
template<typename T>
void fwt(T f[], size_t n) {
  assert(bo::popcnt_u64(n) == 1);
  for (int i = 1; i < n; i <<= 1) {
    for (int j = 0; j < n; j++) {
      if ((i & j) != 0) continue;
      auto x = f[j], y = f[j | i];
      f[j]     = x + y;
      f[j | i] = x - y;
    }
  }
}

template<typename T>
void ifwt(T f[], size_t n) {
  assert(bo::popcnt_u64(n) == 1);
  for (int i = 1; i < n; i <<= 1) {
    for (int j = 0; j < n; j++) {
      if ((i & j) != 0) continue;
      auto x = f[j], y = f[j | i];
      f[j]     = (x + y) / 2;
      f[j | i] = (x - y) / 2;
    }
  }
}

template<typename T>
void index_xor_convolution_for_xcheck(T f[], T Tg[], size_t n) {
  fwt(f, n);
  for (int i = 0; i < n; i++) {
    f[i] *= Tg[i];
  }
  ifwt(f, n);
}

}

#endif //PLAIN_DA_TRIES__CONVOLUTION_HPP_
