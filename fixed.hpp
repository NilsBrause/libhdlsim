/******************************************************************************
 * Copyright (c) 2015, Nils Christopher Brause
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
  typedef typename std::conditional<sign, int64_t, uint64_t>::type word_t;

  // constants
  static const unsigned int word_size = sizeof(word_t)*8-2;
  static const unsigned int words = (bits+word_size-1)/word_size;

  // storage
  std::array<word_t, words> value;

  template <bool sign2, unsigned int mbits2, unsigned int fbits2> friend class fixed_t;

  // add with carry
  inline word_t awc(word_t a, word_t b, bool &carry) const
  {
    word_t c = a + b + carry;
    if(c & power<word_t>(2, word_size))
      {
        carry = true;
        c &= power<word_t>(2, word_size)-1;
      }
    else carry = false;
    return c;
  }

  // extend sign into carry bit (for comparisions)
  inline word_t ext(word_t x) const
  {
    if(sign && x & power<word_t>(2, word_size-1))
      x |= (power<word_t>(2, sizeof(word_t)*8 - word_size)-1) << word_size;
    return x;
  }

  // extend sign to fill the whole word size (execpt carry bit)
  inline void signext()
  {
    if(bits % word_size != 0)
      {
        value[words-1] &= power<word_t>(2, bits % word_size)-1;
        if(sign && at(bits-1))
          value[words-1] |= (power<word_t>(2, word_size-(bits % word_size))-1)
            << (bits % word_size);
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
    for(unsigned int c = 0; c < mbits; c++)
      x *= 0.5;
    assert(x <= 1 && ((sign && x >= -1) || (!sign && x >= 0)));

    bool neg = sign && x < 0;
    if(neg)
        x = -x;
    if(sign)
      set(bits-1, false);

    type d = 0.5;
    for(unsigned int c = sign ? 1 : 0; c < bits; c++)
      {
        unsigned int n = bits-c-1;
        if(d == 0) // due to finite precision of type
          break;
        else if(x >= d)
          {
            set(n, true);
            x -= d;
          }
        else
          set(n, false);
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
    if(sign)
      assert(x <= power<type>(static_cast<type>(2), mbits-1)-1
             && x >= -(power<type>(static_cast<type>(2), mbits-1)-1));
    else
      assert(x <= power<type>(static_cast<type>(2), mbits)-1 && x >= 0);

    bool neg = x < 0;
    if(neg)
      x = -x;
    if(sign)
      set(bits-1, false);

    type i = power<type>(2, mbits - (sign ? 2 : 1));
    for(unsigned int c = sign ? 1 : 0; c < mbits; c++)
      {
        unsigned int n = bits-c-1;
        if(i == 0)
          break;
        else if (x >= i)
          {
            set(n, true);
            x -= i;
          }
        else
          set(n, false);
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
    return *this;
  }

  // access operators

  inline bool at(const unsigned int bit) const
  {
    assert(bit < bits);
    return value[bit/word_size] & power<word_t>(2, bit % word_size);
  }

  inline bool operator[](const unsigned int bit) const
  {
    return at(bit);
  }

  inline void set(const unsigned int bit, bool val)
  {
    assert(bit < bits);
    if(val)
      value[bit/word_size] |= power<word_t>(2, bit % word_size);
    else
      value[bit/word_size] &= ~power<word_t>(2, bit % word_size);
  }

  // conversion operators
  
  explicit operator long double() const
  {
    long double result = 0;
    bool neg = negative();
    fixed_t<sign, mbits, fbits> tmp;
    if(neg)
      tmp = -*this;
    else
      tmp = *this;

    long double d = 0.5;
    for(unsigned int c = (sign ? 1 : 0); c < bits; c++)
      {
        unsigned int n = bits-c-1;
        if(d == 0) // due to finite precision of type
          break;
        else if(tmp.at(n))
          result += d;
        d *= 0.5;
      }

    if(neg)
      {
        if(result == 0)
          result = 1;
        result = -result;
      }

    for(unsigned int c = 0; c < mbits; c++)
      result *= 2;
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
    return operator<<=(static_cast<int>(static_cast<long double>(amount)+.5l));
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
    return *this;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> operator|(const fixed_t<sign2, mbits, fbits> &x) const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = value[c] | x.value[c];
    return tmp;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> &operator&=(const fixed_t<sign2, mbits, fbits> & x)
  {
    for(unsigned int c = 0; c < words; c++)
      value[c] &= x.value[c];
    return *this;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> operator&(const fixed_t<sign2, mbits, fbits> &x) const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = value[c] & x.value[c];
    return tmp;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> &operator^=(const fixed_t<sign2, mbits, fbits> & x)
  {
    for(unsigned int c = 0; c < words; c++)
      value[c] ^= x.value[c];
    return *this;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> operator^(const fixed_t<sign2, mbits, fbits> &x) const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = value[c] ^ x.value[c];
    return tmp;
  }

  fixed_t<sign, mbits, fbits> operator~() const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = ~value[c] & (power<word_t>(2, word_size)-1);
    return tmp;
  }

  inline fixed_t<sign, mbits, fbits> operator!() const
  {
    return ~*this; // TODO: proper implementation
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
    for(unsigned int c = 0; c < words; c++)
      {
        word_t a = ext(value[words-c-1]);
        word_t b = ext(x.value[words-c-1]);
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
    return *this;
  }

  template <bool sign2>
  fixed_t<sign, mbits, fbits> sum(const fixed_t<sign2, mbits, fbits> &x, bool &carry) const
  {
    fixed_t<sign, mbits, fbits> tmp;
    for(unsigned int c = 0; c < words; c++)
      tmp.value[c] = awc(value[c], x.value[c], carry);
    tmp.signext();
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

    const unsigned int extra_bits =
      (words+fixed_t<sign, mbits2, fbits2>::words)*word_size
      - (mbits+mbits2+fbits+fbits2);
    fixed_t<sign, mbits+mbits2+extra_bits, fbits+fbits2> tmp;

    bool carry = false;
    word_t hcarry = 0;
    for(unsigned int c = 0; c < tmp.words-1; c++)
      {
        word_t lsum = 0;
        word_t hsum = hcarry;
        for(unsigned int n = c > x.words-1 ? c-x.words+1 : 0;
            n <= c && n < words; n++)
          {
            unsigned int m = c - n;
            word_t a = value.at(n);
            word_t b = x.value.at(m);

            // split words
            word_t ah = a >> word_size/2 & (power<word_t>(2, word_size/2)-1);
            word_t al = a & (power<word_t>(2, word_size/2)-1);
            word_t bh = b >> word_size/2 & (power<word_t>(2, word_size/2)-1);
            word_t bl = b & (power<word_t>(2, word_size/2)-1);

            // multiply
            word_t ahbh = ah * bh;
            word_t albh = al * bh;
            word_t ahbl = ah * bl;
            word_t albl = al * bl;

            // add
            word_t l = albl;
            word_t h = ahbh;
            l = (l + (albh << word_size/2)) & (power<word_t>(2, word_size)-1);
            h = (h + (albh >> word_size/2)) & (power<word_t>(2, word_size)-1);
            l = (l + (ahbl << word_size/2)) & (power<word_t>(2, word_size)-1);
            h = (h + (ahbl >> word_size/2)) & (power<word_t>(2, word_size)-1);

            lsum = awc(lsum, l, carry);
            hsum = awc(hsum, h, carry);
            if(carry) hcarry++;
            carry = false;
          }
        tmp.value.at(c) = awc(tmp.value.at(c), lsum, carry);
        tmp.value.at(c+1) = awc(tmp.value.at(c+1), hsum, carry);
      }

    tmp <<= sign ? 1 : 0;
    
    return tmp.template resize<mbits+mbits2, fbits+fbits2>();
  }

  template <unsigned int mbits2, unsigned int fbits2>
  fixed_t<sign, mbits+mbits2, fbits+fbits2> operator/(const fixed_t<sign, mbits2, fbits2> &x) const
  {
    return fixed_t<sign, mbits+mbits2, fbits+fbits2>(); // TODO: implement
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
        if(sign && at(bits-1))
          for(unsigned int c = words; c < tmp.words; c++)
            tmp.value[c] = power<word_t>(2, word_size)-1;
        tmp <<= (fbits2-fbits);
      }
    else
      {
        auto tmp2 = *this << (fbits2-fbits);
        for(unsigned int c = 0; c < tmp.words; c++)
          tmp.value[c] = tmp2.value[c];
      }
    return tmp;
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
  return fixed_t<true, mbits, fbits>(sin(2.*pi*static_cast<long double>(p)));
}  

template <bool sign, unsigned int mbits, unsigned int fbits>
fixed_t<true, mbits, fbits> cos(fixed_t<sign, mbits, fbits> p)
{
  return fixed_t<true, mbits, fbits>(cos(2.*pi*static_cast<long double>(p)));
}

template <bool sign, unsigned int mbits, unsigned int fbits>
std::ostream &operator<<(std::ostream &os, const fixed_t<sign, mbits, fbits> &f)
{
  os << static_cast<long double>(f);
  return os;
}

#endif
