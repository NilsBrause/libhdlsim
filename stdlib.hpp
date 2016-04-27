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

#ifndef STDLIB_HPP
#define STDLIB_HPP

#include <array>
#include <type_traits>

#include <wire.hpp>
#include <part.hpp>
#include <fixed.hpp>

namespace hdl
{
  constexpr unsigned int log2ceil(unsigned int x, unsigned int i = 0)
  {
    return power(2u, i) >= x ? i : log2ceil(x, i+1);
  }

  //---------------------------------------------------------------------------

  template <typename T>
  void print(wire<T> w)
  {
    part({ w },
         { },
         [=] (uint64_t time)
         {
           std::cout << "[" << time << "] "
                     << w.getname() << ": " << w << std::endl;
         }, "print");
  }
  
  template <typename T, unsigned int bits>
  void print(bus<T, bits> b)
  {
    part({ b },
         { },
         [=] (uint64_t time)
         {
           std::cout << "[" << time << "] "
                     << b.getname() << ": " << b << std::endl;
         }, "print");
  }

  template <typename U, typename T, unsigned int bits>
  void print(bus<T, bits> b)
  {
    part({ b },
         { },
         [=] (uint64_t time)
         {
           std::cout << "[" << time << "] "
                     << b.getname() << ": " << static_cast<U>(b) << std::endl;
         }, "print");
  }

  template <typename T>
  void assign(wire<T> in,
              wire<T> out)
  {
    part({ in },
         { out },
         [=] (uint64_t)
         {
           out = in;
         }, "assign");
  }

  template <typename T, unsigned int bits>
  void assign(bus<T, bits> in,
              bus<T, bits> out)
  {
    part({ in },
         { out },
         [=] (uint64_t)
         {
           out = in;
         }, "assign");
  }

  template <bool sign1, bool sign2,
            unsigned int mbits, unsigned int fbits>
  typename std::enable_if<sign1 != sign2>::type
  assign(wire<fixed_t<sign1, mbits, fbits>> in,
              wire<fixed_t<sign2, mbits, fbits>> out)
  {
    part({ in },
         { out },
         [=] (uint64_t)
         {
           out = in;
         }, "assign");
  }

  template<typename T, bool sign, unsigned int bits,
           unsigned int mbits, unsigned int fbits>
  typename std::enable_if<mbits+fbits == bits>::type
  assign(bus<T, bits> in,
         wire<fixed_t<sign, mbits, fbits>> out)
  {
    part({ in },
         { out },
         [=] (uint64_t)
         {
           fixed_t<sign, mbits, fbits> tmp;
           for(unsigned int c = 0; c < bits; c++)
             tmp.set(c, in[c]);
           out = tmp;
         }, "assign");
  }

  template<typename T, bool sign, unsigned int bits,
           unsigned int mbits, unsigned int fbits>
  typename std::enable_if<mbits+fbits == bits>::type
  assign(wire<fixed_t<sign, mbits, fbits>> in,
         bus<T, bits> out)
  {
    part({ in },
         { out },
         [=] (uint64_t)
         {
           for(unsigned int c = 0; c < bits; c++)
             in[c] = out[c];
         }, "assign");
  }

  template <typename B, typename T>
  void reg(wire<B> clk,
           wire<B> reset,
           wire<B> enable,
           wire<T> din,
           wire<T> dout)
  {
    part({ clk, reset, enable, din },
         { dout },
         [=] (uint64_t)
         {
           if(reset == static_cast<B>(false))
             dout = T();
           else if(clk.event() and clk == static_cast<B>(true)
                   and enable == static_cast<B>(true))
             dout = din;
         }, "reg");
  }

  template <typename B, typename T, unsigned int bits>
  void reg(wire<B> clk,
           wire<B> reset,
           wire<B> enable,
           bus<T, bits> din,
           bus<T, bits> dout)
  {
    static_assert(bits > 0, "bits > 0");
    for(unsigned int c = 0; c < bits; c++)
      reg(clk, reset, enable, din[c], dout[c]);
  }

  template <unsigned int dt, typename B, typename T>
  void delay(wire<B> clk,
             wire<B> reset,
             wire<B> enable,
             wire<T> din,
             wire<T> dout)
  {
    std::array<wire<T>, dt+1> wires;
    assign(din, wires[0]);
    assign(wires[dt], dout);
    for(unsigned int c = 0; c < dt; c++)
      reg(clk, reset, enable, wires[c], wires[c+1]);
  }

  template <typename T>
  void invert(wire<T> in,
              wire<T> out)
  {
    part({ in },
         { out },
         [=] (uint64_t)
         {
           out = ~in;
         }, "invert");
  }
  
  template <typename T>
  void band(wire<T> in1,
            wire<T> in2,
            wire<T> out)
  {
    part({ in1, in2 },
         { out },
         [=] (uint64_t)
         {
           out = in1 & in2;
         }, "and1");
  }

  template <typename T>
  void bnand(wire<T> in1,
             wire<T> in2,
             wire<T> out)
  {
    wire<T> tmp;
    band(in1, in2, tmp);
    invert(tmp, out);
  }

  template <typename T>
  void bor(wire<T> in1,
           wire<T> in2,
           wire<T> out)
  {
    part({ in1, in2 },
         { out },
         [=] (uint64_t)
         {
           out = in1 | in2;
         }, "or1");
  }

  template <typename T>
  void bnor(wire<T> in1,
            wire<T> in2,
            wire<T> out)
  {
    wire<T> tmp;
    bor(in1, in2, tmp);
    invert(tmp, out);
  }

  template <typename T>
  void bxor(wire<T> in1,
            wire<T> in2,
            wire<T> out)
  {
    part({ in1, in2 },
         { out },
         [=] (uint64_t)
         {
           out = in1 ^ in2;
         }, "xor1");
  }

  template <typename T>
  void bxnor(wire<T> in1,
             wire<T> in2,
             wire<T> out)
  {
    wire<T> tmp;
    bxor(in1, in2, tmp);
    invert(tmp, out);
  }

  template <typename T = bool, bool sign, bool sign2, bool sign3,
            unsigned int mbits, unsigned int fbits>
  void add(wire<fixed_t<sign, mbits, fbits>> in1,
           wire<fixed_t<sign2, mbits, fbits>> in2,
           wire<fixed_t<sign3, mbits, fbits>> out,
           wire<T> carryin = wire<T>(0),
           wire<T> carryout = wire<T>())
  {
    static_assert(mbits + fbits > 0, "mbits + fbits > 0");
    part({ in1, in2, carryin },
         { out, carryout },
         [=] (uint64_t)
         {
           bool carry = carryin;
           out = in1.get().sum(in2.get(), carry);
           carryout = carry;
         }, "add");
  }

  template <unsigned int mbits, unsigned int fbits>
  void negative(wire<fixed_t<true, mbits, fbits>> in,
                wire<fixed_t<true, mbits, fbits>> out)
  {
    part({ in },
         { out },
         [=] (uint64_t)
         {
           out = -in;
         }, "negative");
  }

  template <typename T = bool, bool sign, bool sign2, bool sign3,
            unsigned int mbits, unsigned int fbits>
  void sub(wire<fixed_t<sign, mbits, fbits>> in1,
           wire<fixed_t<sign2, mbits, fbits>> in2,
           wire<fixed_t<sign3, mbits, fbits>> out,
           wire<T> borrowin = wire<T>(0),
           wire<T> borrowout = wire<T>())
  {
    static_assert(mbits + fbits > 0, "mbits + fbits > 0");
    part({ in1, in2, borrowin },
         { out, borrowout },
         [=] (uint64_t)
         {
           T borrow = borrowin;
           out = in1.get().diff(in2.get(), borrow);
           borrowout = borrow;
         }, "sub");
  }

  template <typename B, bool sign, unsigned int mbits, unsigned int fbits>
  void compare(wire<fixed_t<sign, mbits, fbits>> in1,
               wire<fixed_t<sign, mbits, fbits>> in2,
               wire<B> equal,
               wire<B> smaller = wire<B>(),
               wire<B> greater = wire<B>(),
               wire<B> unequal = wire<B>(),
               wire<B> smallerequal = wire<B>(),
               wire<B> greaterequal = wire<B>())
  {
    part({ in1, in2 },
         { equal, smaller, greater, unequal, smallerequal, greaterequal },
         [=] (uint64_t)
         {
           equal = in1 == in2;
           smaller = in1 < in2;
           greater = in2 > in2;
           unequal = in1 != in2;
           smallerequal = in1 <= in2;
           greaterequal = in1 >= in2;
         }, "compare");
  }

  template <bool sign, unsigned int mbits1, unsigned int mbits2, unsigned int fbits1, unsigned int fbits2>
  void resize(wire<fixed_t<sign, mbits1, fbits1>> in,
              wire<fixed_t<sign, mbits2, fbits2>> out)
  {
    part({ in },
         { out },
         [=] (uint64_t)
         {
           out = in.get().template resize<mbits2, fbits2>();
         }, "resize");
  }

  template <bool sign, unsigned int mbits1, unsigned int mbits2, unsigned int fbits1, unsigned int fbits2>
  void round(wire<fixed_t<sign, mbits1, fbits1>> in,
             wire<fixed_t<sign, mbits2, fbits2>> out)
  {
    // TODO: real rounding
    resize(in, out);
  }

  template <bool sign, unsigned int mbits, unsigned int fbits>
  void barrel_shift_fixed(wire<fixed_t<sign, mbits, fbits>> input,
                          int amount,
                          wire<fixed_t<sign, mbits, fbits>> output)
  {
    part({ input },
         { output },
         [=] (uint64_t)
         {
           output = input << amount;
         }, "barrel_shift_fixed");
  }

  template <bool sign, unsigned int mbits, unsigned int fbits, bool sign2>
  void barrel_shift(wire<fixed_t<sign, mbits, fbits>> input,
                    wire<fixed_t<sign2, log2ceil(mbits+fbits)+1, 0>> amount,
                    wire<fixed_t<sign, mbits, fbits>> output)
  {
    part({ input, amount },
         { output },
         [=] (uint64_t)
         {
           output = input << amount;
         }, "barrel_shift");
  }

  template <bool sign, unsigned int mbits, unsigned int fbits,
            unsigned int mbits2, unsigned int fbits2>
  void mul(wire<fixed_t<sign, mbits, fbits>> in1,
           wire<fixed_t<sign, mbits2, fbits2>> in2,
           wire<fixed_t<sign, mbits+mbits2, fbits+fbits2>> out)
  {
    part({ in1, in2 },
         { out },
         [=] (uint64_t)
         {
           out = in1 * in2;
         }, "mul");
  }

  template <typename B, bool sign, unsigned int mbits, unsigned int fbits>
  void integrator(wire<B> clk,
                  wire<B> reset,
                  wire<B> enable,
                  wire<fixed_t<sign, mbits, fbits>> in,
                  wire<fixed_t<sign, mbits, fbits>> out)
  {
    wire<fixed_t<sign, mbits, fbits>> tmp;
    add(out, in, tmp);
    reg(clk, reset, enable, tmp, out);
  }

  template <typename B, bool sign, unsigned int mbits, unsigned int fbits>
  void differentiator(wire<B> clk,
                      wire<B> reset,
                      wire<B> enable,
                      wire<fixed_t<sign, mbits, fbits>> in,
                      wire<fixed_t<sign, mbits, fbits>> out)
  {
    wire<fixed_t<sign, mbits, fbits>> tmp;
    reg(clk, reset, enable, in, tmp);
    sub(in, tmp, out);
  }

  template <typename B, bool sign, unsigned int bits>
  void counter(wire<B> clk,
               wire<B> reset,
               wire<B> enable,
               wire<fixed_t<sign, bits, 0>> out)
  {
    wire<fixed_t<sign, bits, 0>> one(1u);
    integrator(clk, reset, enable, one, out);
  }

  template <bool usep, bool usei, bool used,
            unsigned int int_mbits, unsigned int int_fbits,
            bool sign, unsigned int mbits, unsigned int fbits,
            typename B>
  void pidctl(wire<B> clk,
              wire<B> reset,
              wire<B> enable,
              wire<fixed_t<sign, mbits, fbits>> input,
              wire<fixed_t<true, log2ceil(int_mbits+int_fbits)+1, 0>> pgain,
              wire<fixed_t<true, log2ceil(int_mbits+int_fbits)+1, 0>> igain,
              wire<fixed_t<true, log2ceil(int_mbits+int_fbits)+1, 0>> dgain,
              wire<fixed_t<sign, int_mbits, int_fbits>> output)
  {
    // resize input ti int_*bits
    wire<fixed_t<sign, int_mbits, int_fbits>> input2;
    resize(input, input2);

    // gains
    wire<fixed_t<sign, int_mbits, int_fbits>> inputp, inputi, inputd;
    barrel_shift(input2, pgain, inputp);
    barrel_shift(input2, igain, inputi);
    barrel_shift(input2, dgain, inputd);

    // controller
    wire<fixed_t<sign, int_mbits, int_fbits>> resultp, resulti, resultd;
    reg(clk, reset, enable, inputp, resultp);            // P
    integrator(clk, reset, enable, inputi, resulti);     // I
    differentiator(clk, reset, enable, inputd, resultd); // D

    // add
    if(usep && !usei && !used)
      assign(resultp, output);
    else if(!usep && usei && !used)
      assign(resulti, output);
    else if(!usep && !usei && used)
      assign(resultd, output);
    else if(usep && usei && !used)
      add(resultp, resulti, output);
    else if(usep && !usei && used)
      add(resultp, resultd, output);
    else if(!usep && usei && used)
      add(resulti, resultd, output);
    else if(usep && usei && used)
      {
        wire<fixed_t<sign, int_mbits, int_fbits>> tmp;
        add(resultp, resulti, tmp);
        add(resultd, tmp, output);
      }
  }

  template<bool sign, unsigned int phase_mbits, unsigned int phase_fbits,
           unsigned int mbits, unsigned int fbits>
  void sincos(wire<fixed_t<sign, phase_mbits, phase_fbits>> phase,
              wire<fixed_t<true, mbits, fbits>> sin_out,
              wire<fixed_t<true, mbits, fbits>> cos_out)
  {
    part({ phase },
         { sin_out, cos_out },
         [=] (uint64_t)
         {
           sin_out = sin(phase.get()).template resize<mbits, fbits>();
           cos_out = cos(phase.get()).template resize<mbits, fbits>();
         }, "sincos");
  }

  template<typename B, unsigned int freq_bits,
           unsigned int mbits, unsigned int fbits>
  void nco(wire<B> clk,
           wire<B> reset,
           wire<B> enable,
           wire<fixed_t<false, 0, freq_bits>> freq, // f/fs
           wire<fixed_t<false, 0, freq_bits>> mod,
           wire<fixed_t<true, mbits, fbits>> sine,
           wire<fixed_t<true, mbits, fbits>> cosine,
           wire<fixed_t<false, 0, freq_bits>> saw)
  {
    wire<fixed_t<false, 0, freq_bits>> phase, phase2;
    integrator(clk, reset, enable, freq, phase);
    add(phase, mod, phase2);
    sincos(phase2, sine, cosine);
    resize(phase2, saw);
  }

  template<typename B, unsigned int mbits, unsigned int fbits, unsigned int freq_bits>
  void iq_demod(wire<B> clk,
                wire<B> reset,
                wire<B> enable,
                wire<fixed_t<true, mbits, fbits>> input,
                wire<fixed_t<false, 0, freq_bits>> freq, // f/fs
                wire<fixed_t<true, 2*mbits, 2*fbits>> i,
                wire<fixed_t<true, 2*mbits, 2*fbits>> q,
                wire<fixed_t<false, 0, freq_bits>> phase)
  {
    wire<fixed_t<true, mbits, fbits>> sine, cosine;
    nco(clk,
        reset,
        enable,
        freq,
        wire<fixed_t<false, 0, freq_bits>>(0.),
        sine,
        cosine,
        phase);
    mul(input, sine, i);
    mul(input, cosine, q);
  }

  template<unsigned int int_mbits, unsigned int int_fbits,
           typename B, unsigned int mbits, unsigned int fbits, unsigned int freq_bits>
  void pll(wire<B> clk,
           wire<B> reset,
           wire<B> enable,
           wire<fixed_t<true, mbits, fbits>> input,
           wire<fixed_t<false, 0, freq_bits>> freq_start, // f/fs
           wire<fixed_t<true, log2ceil(int_mbits+int_fbits)+1, 0>> pgain,
           wire<fixed_t<true, log2ceil(int_mbits+int_fbits)+1, 0>> igain,
           wire<fixed_t<false, 0, freq_bits>> freq_out, // f/fs
           wire<fixed_t<true, 2*mbits, 2*fbits>> i,
           wire<fixed_t<true, 2*mbits, 2*fbits>> q,
           wire<fixed_t<true, 2*mbits, 2*fbits>> error)
  {
    iq_demod(clk,
             reset,
             enable,
             input,
             freq_out,
             i,
             q,
             wire<fixed_t<false, 0, freq_bits>>());

    wire<fixed_t<true, int_mbits, int_fbits>> pidout;
    pidctl<true, true, false, int_mbits, int_fbits>
      (clk, reset, enable, error, pgain, igain,
       wire<fixed_t<true, log2ceil(int_mbits+int_fbits)+1, 0>>(0), pidout);

    wire<fixed_t<true, 0, freq_bits>> pidout2;
    resize(pidout, pidout2);
    add(pidout2, freq_start, freq_out);
  }

  template<unsigned int n, unsigned int r, bool sign,
           unsigned int mbits, unsigned int in_fbits,
           unsigned int out_fbits, typename B>
  void cic_down(wire<B> clk,
                wire<B> clk2,
                wire<B> reset,
                wire<B> enable,
                wire<fixed_t<sign, mbits, in_fbits>> input,
                wire<fixed_t<sign, mbits, out_fbits>> output)
  {
    const unsigned int fbits2 = n*r + in_fbits;
    wire<fixed_t<sign, mbits, fbits2>> input2;
    std::array<wire<fixed_t<sign, mbits, fbits2>>, n+1> ints;
    wire<fixed_t<sign, mbits, out_fbits>> tmp;
    std::array<wire<fixed_t<sign, mbits, out_fbits>>, n+1> combs;

    resize(input, input2);
    barrel_shift_fixed(input2, -static_cast<int>(n*r), ints.at(0));

    for(unsigned int c = 0; c < n; c++)
      integrator(clk,
                 reset,
                 enable,
                 ints.at(c),
                 ints.at(c+1));

    resize(ints.at(n), tmp);

    reg(clk2,
        reset,
        enable,
        tmp,
        combs.at(0));

    for(unsigned int c = 0; c < n; c++)
      differentiator(clk2,
                     reset,
                     enable,
                     combs.at(c),
                     combs.at(c+1));

    resize(combs.at(n), output);
  }

  template<unsigned int n, unsigned int r, bool sign,
           unsigned int mbits, unsigned int in_fbits,
           unsigned int out_fbits, typename B>
  void cic_up(wire<B> clk,
              wire<B> clk2,
              wire<B> reset,
              wire<B> enable,
              wire<fixed_t<sign, mbits, in_fbits>> input,
              wire<fixed_t<sign, mbits, out_fbits>> output)
  {
    std::array<wire<fixed_t<sign, mbits, in_fbits>>, n+1> combs;
    const unsigned int fbits2 = n*r + in_fbits;
    wire<fixed_t<sign, mbits, fbits2>> tmp;
    wire<fixed_t<sign, mbits, fbits2>> tmp2;
    std::array<wire<fixed_t<sign, mbits, fbits2>>, n+1> ints;

    assign(input, combs.at(0));

    for(unsigned int c = 0; c < n; c++)
      differentiator(clk,
                     reset,
                     enable,
                     combs.at(c),
                     combs.at(c+1));

    resize(combs.at(n), tmp);
    barrel_shift_fixed(tmp, -static_cast<int>(n*r), tmp2);

    reg(clk2,
        reset,
        enable,
        tmp2,
        ints.at(0));

    for(unsigned int c = 0; c < n; c++)
      integrator(clk2,
                 reset,
                 enable,
                 ints.at(c),
                 ints.at(c+1));

    resize(ints.at(n), output);
  }

  template<unsigned int bits,
           typename T>
  void pwm(wire<T> clk,
           wire<T> reset,
           wire<T> enable,
           wire<fixed_t<false, log2ceil(bits+1), 0>> ratio,
           wire<T> output)
  {
    wire<fixed_t<false, log2ceil(bits+1), 0>> cnt_out;
    wire<T> cnt_rst;

    counter(clk,
            cnt_rst,
            enable,
            cnt_out);

    wire<T> equal;
    compare(cnt_out,
            wire<fixed_t<false, log2ceil(bits+1), 0>>(bits),
            equal);

    wire<T> nreset;
    invert(reset, nreset);
    bnor(equal, nreset, cnt_rst);

    compare(cnt_out, ratio, wire<T>(), output);
  }

  template<unsigned int div,
           typename B>
  void clkdiv(wire<B> clk,
              wire<B> reset,
              wire<B> enable,
              wire<B> clk_out)
  {
    wire<fixed_t<false, log2ceil(div+1), 0>> ratio(div/2);
    pwm<div>(clk,
             reset,
             enable,
             ratio,
             clk_out);
  }
}

#endif
