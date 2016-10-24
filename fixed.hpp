/******************************************************************************
 * Copyright (c) 2015-2016, Nils Christopher Brause
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef FIXED_HPP
#define FIXED_HPP

#include <array>
#include <cassert>
#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

// cake
const long double pi = std::acos(-1.l);

// std::pow at compile-time
template <typename type>
constexpr type power(const type base, const unsigned long int exp)
{
  return !exp ? 1 : base * power(base, exp-1);
}

template <typename type>
constexpr unsigned long int logceil(const type base, const type val)
{
  return val > 1 ? 1ul + logceil(base, val/base) : 0;
}

// fixed point arithmetic class
template <bool sign, unsigned int mbits, unsigned int fbits>
class fixed_t
{
private:
  const static unsigned int bits = mbits + fbits;
  static_assert(bits > 0, "mbits + fbits must be non-zero.");

  // choose appropriate type
  typedef uintmax_t word_t;

  // constants
  static const unsigned int word_size = sizeof(word_t)*8;
  static const unsigned int words = (bits+word_size-1)/word_size; // round up

  // storage
  std::array<word_t, words> value;

  template <bool sign2, unsigned int mbits2, unsigned int fbits2> friend class fixed_t;

  // add with carry
  inline word_t awc(word_t a, word_t b, bool &carry) const
  {
    if(carry)
      {
        if(a != word_t(-1))
          a += 1;
        else if(b != word_t(-1))
          b += 1;
        else
          return word_t(-2);
        carry = false;
      }
    if(word_t(-1) - a < b)
      carry = true;
    else
      carry = false;
    return a + b;
  }

  // multiply
  inline void mul(word_t a, word_t b, word_t &h, word_t &l) const
  {
    word_t al = a & (power<word_t>(2, word_size/2)-1);
    word_t ah = (a >> word_size/2) & (power<word_t>(2, word_size/2)-1);
    word_t bl = b & (power<word_t>(2, word_size/2)-1);
    word_t bh = (b >> word_size/2) & (power<word_t>(2, word_size/2)-1);

    word_t albl = al * bl;
    word_t albh = al * bh;
    word_t ahbl = ah * bl;
    word_t ahbh = ah * bh;

    bool carry = false;
    word_t hcarry = 0;

    l = albl;
    l = awc(l, albh << word_size/2, carry);
    hcarry += carry;
    carry = false;
    l = awc(l, ahbl << word_size/2, carry);
    hcarry += carry;
    carry = false;

    h = awc(ahbh, hcarry, carry);
    carry = false;
    h = awc(h, albh >> word_size/2, carry);
    carry = false;
    h = awc(h, ahbl >> word_size/2, carry);
  }

  // extend sign to fill the whole word size
  inline void signext()
  {
    // mask for extra bits in most significant word.
    word_t mask = (power<word_t>(2, word_size-(bits % word_size))-1) << (bits % word_size);
    if(bits % word_size != 0)
      {
        if(sign)
          {
            if(negative())
              value[words-1] |= mask;
            else
              value[words-1] &= ~mask;
          }
        else
          value[words-1] &= ~mask;
      }
  }

public:
  // constructors

  fixed_t()
  {
    for(auto &word : value)
      word = 0;
  }

  template<typename type>
  fixed_t(type x, typename std::enable_if<std::is_floating_point<type>::value, fixed_t<sign, mbits, fbits>>::type *t = NULL)
  {
    if(!sign)
      assert(x >= 0);

    bool neg = sign && x < 0;
    if(neg)
        x = -x;

    assert(x < std::pow<type>(2, mbits - (sign ? 1 : 0)));

    for(unsigned int c = 0; c < words; c++)
      value[c] = 0;

    type d;
    if((sign && mbits >= 2) || (!sign && mbits >= 1))
      d = std::pow<type>(2, mbits - (sign ? 2 : 1));
    else
      d = 0.5;
    for(unsigned int c = 0; c < bits - (sign ? 1 : 0); c++)
      {
        unsigned int n = bits-c-1;
        if(x == 0 || d == 0)
          break;
        else if(x >= d)
          {
            set(n, true);
            x -= d;
          }
        d *= 0.5;
      }

    if(neg)
      *this = -*this;
    signext();
  }

  template <typename type>
  fixed_t(type x, typename std::enable_if<std::is_integral<type>::value &&
          ((std::is_signed<type>::value && sign) || (std::is_unsigned<type>::value && !sign)),
          fixed_t<sign, mbits, fbits>>::type *t = NULL)
  {
    bool neg = sign && x < 0;
    if(neg)
        x = -x;

    assert(x < power<type>(2, mbits - (sign ? 1 : 0)));

    for(unsigned int c = 0; c < words; c++)
      value[c] = 0;

    type i = power<type>(2, mbits - (sign ? 2 : 1));
    for(unsigned int c = 0; c < bits - (sign ? 1 : 0); c++)
      {
        unsigned int n = bits-c-1;
        if(x == 0 || i == 0)
          break;
        else if (x >= i)
          {
            set(n, true);
            x -= i;
          }
        i /= 2;
      }

    if(neg)
      *this = -*this;
    signext();
  }

  // assignment operator

  template <bool sign2>
  fixed_t<sign, mbits, fbits> &operator=(const fixed_t<sign2, mbits, fbits> x)
  {
    for(unsigned int c = 0; c < words; c++)
      value[c] = x.value[c];
    signext();
    return *this;
  }

  // access operators

  inline bool at(const unsigned int bit) const
  {
    assert(bit < bits);
    return value[bit/word_size] & (static_cast<word_t>(1) << (bit % word_size));
  }

  inline bool operator[](const unsigned int bit) const
  {
    return at(bit);
  }

  inline void set(const unsigned int bit, bool val)
  {
    assert(bit < bits);
    word_t pattern = static_cast<word_t>(1) << (bit % word_size);
    if(val)
      value[bit/word_size] |= pattern;
    else
      value[bit/word_size] &= ~pattern;
    if(bit == bits-1)
      signext();
  }

  // conversion operators

  explicit operator long double() const
  {
    bool neg = negative();
    fixed_t<sign, mbits, fbits> tmp;
    if(neg)
      tmp = -*this;
    else
      tmp = *this;

    long double result = 0;
    long double d;
    if((sign && mbits >= 2) || (!sign && mbits >= 1))
      d = std::pow<long double>(2, mbits - (sign ? 2 : 1));
    else
      d = 0.5;
    for(unsigned int c = 0; c < bits - (sign ? 1 : 0); c++)
      {
        unsigned int n = bits-c-1;
        if(d == 0) // due to finite precision of type
          break;
        else if(tmp.at(n))
          result += d;
        d *= 0.5l;
      }

    if(neg)
      result = -result;

    return result;
  }

  inline explicit operator double() const
  {
    return static_cast<double>(operator long double());
  }

  inline explicit operator float() const
  {
    return static_cast<float>(operator long double());
  }

  // logic operators

  template <unsigned int mbits2>
  inline fixed_t<sign, mbits, fbits> &operator<<=(const fixed_t<true, mbits2, 0> amount)
  {
    return operator<<=(static_cast<int>(static_cast<long double>(amount)));
  }

  fixed_t<sign, mbits, fbits> &operator<<=(const int amount)
  {
    if(amount < 0)
      return operator>>=(-amount);

    unsigned int amount_words = amount / word_size;
    unsigned int amount_bits = amount % word_size;

    for(unsigned int c = 0; c < words; c++)
      {
        unsigned int d = words-c-1;
        value[d] = d >= amount_words ?
          value[d-amount_words] << amount_bits : 0;
        value[d] |= d > amount_words ?
          value[d-amount_words-1] >> (word_size - amount_bits) : 0;
        value[d] &= power<word_t>(2, word_size)-1;
      }

#ifdef SYMMETRIC
    if(asymmetric())
      set(0, true);
#endif

    return *this;
  }

  template <unsigned int mbits2>
  inline fixed_t<sign, mbits, fbits> operator<<(const fixed_t<true, mbits2, 0> amount) const
  {
    return operator<<(static_cast<int>(static_cast<long double>(amount)+.5l));
  }

  fixed_t<sign, mbits, fbits> operator<<(const int amount) const
  {
    if(amount < 0)
      return operator>>(-amount);

    fixed_t<sign, mbits, fbits> tmp;

    unsigned int amount_words = amount / word_size;
    unsigned int amount_bits = amount % word_size;

    for(unsigned int c = 0; c < words; c++)
      {
        unsigned int d = words-c-1;
        tmp.value[d] = d >= amount_words ?
          value[d-amount_words] << amount_bits : 0;
        tmp.value[d] |= d > amount_words ?
          value[d-amount_words-1] >> (word_size - amount_bits) : 0;
        tmp.value[d] &= power<word_t>(2, word_size)-1;
      }

#ifdef SYMMETRIC
    if(tmp.asymmetric())
      tmp.set(0, true);
#endif

    return tmp;
  }

  template <unsigned int mbits2>
  inline fixed_t<sign, mbits, fbits> &operator>>=(const fixed_t<true, mbits2, 0> amount)
  {
    return operator>>=(static_cast<int>(static_cast<long double>(amount)+.5l));
  }

  fixed_t<sign, mbits, fbits> &operator>>=(const int amount)
  {
    if(amount < 0)
      return operator<<=(-amount);

    bool neg = negative();

    unsigned int amount_words = amount / word_size;
    unsigned int amount_bits = amount % word_size;

    for(unsigned int c = 0; c < words; c++)
      {
        word_t fill = neg ? power<word_t>(2, word_size)-1 : 0;
        value[c] = (c+amount_words < words) ?
          value[c+amount_words] >> amount_bits : fill;
        value[c] |= (c+amount_words < words-1 ? value[c+amount_words+1] : fill)
          << (word_size - amount_bits);
        value[c] &= power<word_t>(2, word_size)-1;
      }

    signext();
    return *this;
  }

  template <unsigned int mbits2>
  inline fixed_t<sign, mbits, fbits> operator>>(const fixed_t<true, mbits2, 0> amount) const
  {
    return operator>>(static_cast<int>(static_cast<long double>(amount)+.5l));
  }

  fixed_t<sign, mbits, fbits> operator>>(const int amount) const
  {
    if(amount < 0)
      return operator<<(-amount);

    fixed_t<sign, mbits, fbits> tmp;

    bool neg = negative();

    unsigned int amount_words = amount / word_size;
    unsigned int amount_bits = amount % word_size;

    for(unsigned int c = 0; c < words; c++)
      {
        word_t fill = neg ? power<word_t>(2, word_size)-1 : 0;
        tmp.value[c] = (c+amount_words < words) ?
          value[c+amount_words] >> amount_bits : fill;
        tmp.value[c] |= (c+amount_words < words-1 ?
                         value[c+amount_words+1] : fill)
          << (word_size - amount_bits);
        tmp.value[c] &= power<word_t>(2, word_size)-1;
      }

    tmp.signext();
    return tmp;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> &operator|=(const fixed_t<sign2, mbits, fbits> & x)
  {
    for(unsigned int c = 0; c < words; c++)
      value[c] |= x.value[c];
    signext();
    return *this;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> operator|(const fixed_t<sign2, mbits, fbits> &x) const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = value[c] | x.value[c];
    tmp.signext();
    return tmp;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> &operator&=(const fixed_t<sign2, mbits, fbits> & x)
  {
    for(unsigned int c = 0; c < words; c++)
      value[c] &= x.value[c];
    signext();
    return *this;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> operator&(const fixed_t<sign2, mbits, fbits> &x) const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = value[c] & x.value[c];
    tmp.signext();
    return tmp;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> &operator^=(const fixed_t<sign2, mbits, fbits> & x)
  {
    for(unsigned int c = 0; c < words; c++)
      value[c] ^= x.value[c];
    signext();
    return *this;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> operator^(const fixed_t<sign2, mbits, fbits> &x) const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = value[c] ^ x.value[c];
    tmp.signext();
    return tmp;
  }

  fixed_t<sign, mbits, fbits> operator~() const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = ~value[c] & (power<word_t>(2, word_size)-1);
    tmp.signext();
    return tmp;
  }

  inline fixed_t<sign, mbits, fbits> operator!() const
  {
    if(zero())
      return 1;
    else
      return 0;
  }

  // comparison operators

  bool operator==(const fixed_t<sign, mbits, fbits> &x) const
  {
    bool equal = true;
    for(unsigned int c = 0; c < words; c++)
      if(value[c] != x.value[c])
        {
          equal = false;
          break;
        }
    return equal;
  }

  bool operator>(const fixed_t<sign, mbits, fbits> &x) const
  {
    if(negative() && !x.negative())
      return false;
    else if(!negative() && x.negative())
      return true;
    for(unsigned int c = 0; c < words; c++)
      {
        word_t a = value[words-c-1];
        word_t b = x.value[words-c-1];
        if(a > b)
          return true;
        else if(a < b)
          return false;
      }
    return false;
  }

  inline bool operator!=(const fixed_t<sign, mbits, fbits> &x) const
  {
    return !(*this == x);
  }

  inline bool operator<(const fixed_t<sign, mbits, fbits> &x) const
  {
    return x > *this;
  }

  inline bool operator>=(const fixed_t<sign, mbits, fbits> &x) const
  {
    return *this > x || *this == x;
  }

  inline bool operator<=(const fixed_t<sign, mbits, fbits> &x) const
  {
    return *this < x || *this == x;
  }

  // arithmetic operators

  template <bool sign2>
  fixed_t<sign, mbits, fbits> &add(const fixed_t<sign2, mbits, fbits> &x, bool &carry)
  {
    for(unsigned int c = 0; c < words; c++)
      value[c] = awc(value[c], x.value[c], carry);
    signext();

#ifdef SYMMETRIC
    if(asymmetric())
      set(0, true);
#endif

    return *this;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> sum(const fixed_t<sign2, mbits, fbits> &x, bool &carry) const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = awc(value[c], x.value[c], carry);
    tmp.signext();

#ifdef SYMMETRIC
    if(tmp.asymmetric())
      tmp.set(0, true);
#endif

    return tmp;
  }

  template <bool sign2>
  inline fixed_t<sign, mbits, fbits> &sub(const fixed_t<sign2, mbits, fbits> &x, bool &borrow)
  {
    fixed_t<sign, mbits, fbits> nx = ~x;
    borrow = !borrow;
    return add(nx, borrow);
  }

  template <bool sign2>
  inline fixed_t<sign, mbits, fbits> diff(const fixed_t<sign2, mbits, fbits> &x, bool &borrow) const
  {
    fixed_t<sign, mbits, fbits> nx = ~x;
    borrow = !borrow;
    return sum(nx, borrow);
  }

  template <bool sign2>
  inline fixed_t<sign, mbits, fbits> &operator+=(const fixed_t<sign2, mbits, fbits> &x)
  {
    bool carry = false;
    return add(x, carry);
  }

  template <bool sign2>
  inline fixed_t<sign, mbits, fbits> operator+(const fixed_t<sign2, mbits, fbits> &x) const
  {
    bool carry = false;
    return sum(x, carry);
  }

  template <bool sign2>
  inline fixed_t<sign, mbits, fbits> &operator-=(const fixed_t<sign2, mbits, fbits> &x)
  {
    bool borrow = false;
    return sub(x, borrow);
  }

  template <bool sign2>
  inline fixed_t<sign, mbits, fbits> operator-(const fixed_t<sign2, mbits, fbits> &x) const
  {
    bool borrow = false;
    return diff(x, borrow);
  }

  inline fixed_t<sign, mbits, fbits> operator+() const
  {
    return *this;
  }

  // TODO somehow disable for sign
  inline fixed_t<sign, mbits, fbits> operator-() const
  {
    assert(sign);
    if(asymmetric())
      return ~*this;
    static fixed_t<sign, mbits, fbits> zero;
    bool borrow = false;
    return zero.diff(*this, borrow);
  }

  template <unsigned int mbits2, unsigned int fbits2>
  fixed_t<sign, mbits+mbits2, fbits+fbits2> operator*(const fixed_t<sign, mbits2, fbits2> &x) const
  {
    if(negative() && x.negative())
      return -*this * -x;
    else if(!negative() && x.negative())
      return -(*this * -x);
    else if(negative() && !x.negative())
      return -(-*this * x);

    std::array<word_t, 2*words> result;
    std::array<word_t, 2*words> carrys;
    for(unsigned int c = 0; c < 2*words; c++)
      {
        result[c] = 0;
        carrys[c] = 0;
      }

    for(unsigned int n = 0; n < words; n++)
      for(unsigned int m = 0; m < x.words; m++)
        {
          word_t a = value.at(n);
          word_t b = x.value.at(m);
          word_t h;
          word_t l;
          mul(a, b, h, l);
          bool carry = false;
          result[n+m] = awc(result[n+m], l, carry);
          carrys[n+m+1] += carry;
          carry = false;
          result[n+m+1] = awc(result[n+m+1], h, carry);
          carrys[n+m+2] += carry;
        }

    fixed_t<sign, mbits+mbits2, fbits+fbits2> tmp2;
    bool carry = false;
    for(unsigned int c = 0; c < tmp2.words; c++)
      tmp2.value[c] = awc(result[c], carrys[c], carry);

    if(sign)
      tmp2 >>= 1;

#ifdef SYMMETRIC
    if(tmp2.asymmetric())
      tmp2.set(0, true);
#endif

    return tmp2;
  }

  template <unsigned int mbits2, unsigned int fbits2>
  fixed_t<sign, mbits+mbits2, fbits+fbits2> operator/(const fixed_t<sign, mbits2, fbits2> &x) const
  {
    return fixed_t<sign, mbits+mbits2, fbits+fbits2>(); // TODO: implement
  }

  template <unsigned int mbits2, unsigned int fbits2>
  fixed_t<sign, mbits, fbits> &operator*=(const fixed_t<sign, mbits2, fbits2> &x)
  {
    *this = (*this * x).resize<mbits, fbits>();
    return *this;
  }

  // misc

  inline const unsigned int size() const
  {
    return bits;
  }

  inline bool negative() const
  {
    return at(bits-1) && sign;
  }

  inline bool zero() const
  {
    bool itis = true;
    for(auto &word : value)
      if(word != 0)
        {
          itis = false;
          break;
        }
    return itis;
  }

  inline bool asymmetric() const
  {
    if(!negative())
      return false;
    for(unsigned int c = 0; c < bits-2; c++)
      if(at(bits-c-2)) return false;
    return true;
  }

  template <unsigned int mbits2, unsigned int fbits2>
  fixed_t<sign, mbits2, fbits2> resize() const
  {
    fixed_t<sign, mbits2, fbits2> tmp;

    unsigned int bits2 = mbits2+fbits2;
    if(bits2 == bits)
      {
        for(unsigned int c = 0; c < words; c++)
          tmp.value[c] = value[c];
        tmp <<= (fbits2-fbits);
      }
    else if(bits2 > bits)
      {
        for(unsigned int c = 0; c < words; c++)
          tmp.value[c] = value[c];
        tmp <<= (fbits2-fbits);
      }
    else
      {
        auto tmp2 = *this << (fbits2-fbits);
        for(unsigned int c = 0; c < tmp.words; c++)
          tmp.value[c] = tmp2.value[c];
      }
    tmp.signext();

#ifdef SYMMETRIC
    if(fbits2 < fbits && tmp.asymmetric())
      tmp.set(0, true);
#endif

    return tmp;
  }

  template <unsigned int mbits2, unsigned int fbits2>
  inline void resize(fixed_t<sign, mbits2, fbits2> &target)
  {
    target = resize<mbits2, fbits2>();
  }

  std::string bin() const
  {
    std::string result;
    for(unsigned int c = 0; c < bits; c++)
      result += at(bits-c-1) ? '1' : '0';
    return result;
  }
};

template <bool sign, unsigned int mbits, unsigned int fbits>
fixed_t<true, mbits, fbits> sin(fixed_t<sign, mbits, fbits> p)
{
  return fixed_t<true, mbits, fbits>(0.5*sin(2.*pi*static_cast<long double>(p)));
}

template <bool sign, unsigned int mbits, unsigned int fbits>
fixed_t<true, mbits, fbits> cos(fixed_t<sign, mbits, fbits> p)
{
  return fixed_t<true, mbits, fbits>(0.5*cos(2.*pi*static_cast<long double>(p)));
}

template <bool sign, unsigned int mbits, unsigned int fbits>
std::ostream &operator<<(std::ostream &os, const fixed_t<sign, mbits, fbits> &f)
{
  os << static_cast<long double>(f);
  return os;
}

#endif
