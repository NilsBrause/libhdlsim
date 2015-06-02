#ifndef STDLIB_HPP
#define STDLIB_HPP

#include <array>
#include <type_traits>

#include <wire.hpp>
#include <part.hpp>
#include <std_logic.hpp>

namespace hdl
{
  constexpr unsigned int log2ceil(unsigned int x, unsigned int i = 0)
  {
    return power(2u, i) >= x ? i : log2ceil(x, i+1);
  }

  //---------------------------------------------------------------------------

  template <typename T>
  void print(wire<T> w, std::string name)
  {
    part({ w },
         { },
         [=]
         {
           std::cout << "[" << waitfor(0) << "] "
                     << name << ": " << w << std::endl;
         }, "print");
  }
  
  template <typename T, unsigned int bits>
  void print(bus<T, bits> b, std::string name)
  {
    part({ b },
         { },
         [=]
         {
           std::cout << "[" << waitfor(0) << "] "
                     << name << ": " << b << std::endl;
         }, "print");
  }

  template <typename T>
  void assign(wire<T> in,
              wire<T> out)
  {
    part({ in },
         { out },
         [=]
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
         [=]
         {
           out = in;
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
         [=]
         {
           if(reset == static_cast<B>(0))
             dout = 0;
           else if(clk.event() and clk == static_cast<B>(1)
                   and enable == static_cast<B>(1))
             dout = din;
         }, "reg");
  };

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
  };

  template <typename T>
  void invert(wire<T> in,
              wire<T> out)
  {
    part({ in },
         { out },
         [=]
         {
           out = !in;
         }, "invert");
  }
  
  template <typename T>
  void band(wire<T> in1,
            wire<T> in2,
            wire<T> out)
  {
    part({ in1, in2 },
         { out },
         [=]
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
         [=]
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
         [=]
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

  template <typename T, unsigned int bits>
  void invert(bus<T, bits> in,
              bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    for(unsigned int c = 0; c < bits; c++)
      invert(in[c], out[c]);
  }

  template <typename T, unsigned int bits>
  void band(bus<T, bits> in1,
            bus<T, bits> in2,
            bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    for(unsigned int c = 0; c < bits; c++)
      band(in1[c], in2[c], out[c]);
  }

  template <typename T, unsigned int bits>
  void bnand(bus<T, bits> in1,
             bus<T, bits> in2,
             bus<T, bits> out)
  {
    bus<T, bits> tmp;
    band(in1, in2, tmp);
    invert(tmp, out);
  }

  template <typename T, unsigned int bits>
  void bor(bus<T, bits> in1,
           bus<T, bits> in2,
           bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    for(unsigned int c = 0; c < bits; c++)
      bor(in1[c], in2[c], out[c]);
  }

  template <typename T, unsigned int bits>
  void bnor(bus<T, bits> in1,
            bus<T, bits> in2,
            bus<T, bits> out)
  {
    bus<T, bits> tmp;
    bor(in1, in2, tmp);
    invert(tmp, out);
  }

  template <typename T, unsigned int bits>
  void bxor(bus<T, bits> in1,
            bus<T, bits> in2,
            bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    for(unsigned int c = 0; c < bits; c++)
      bxor(in1[c], in2[c], out[c]);
  }

  template <typename T, unsigned int bits>
  void bxnor(bus<T, bits> in1,
             bus<T, bits> in2,
             bus<T, bits> out)
  {
    bus<T, bits> tmp;
    bxor(in1, in2, tmp);
    invert(tmp, out);
  }

  template <typename T, unsigned int bits>
  void add(bus<T, bits> in1,
           bus<T, bits> in2,
           bus<T, bits> out,
           wire<T> carryin = wire<T>(0),
           wire<T> carryout = wire<T>(0))
  {
    static_assert(bits > 0, "bits > 0");
    part({ in1, in2, carryin },
         { out, carryout },
         [=]
         {
           T carry = carryin;
           for(unsigned int c = 0; c < bits; c++)
             {
               out[c] = carry ^ (in1[c] ^ in2[c]);
               carry = (in1[c] & in2[c])
                 | (carry & (in1[c] | in2[c]));
             }
           carryout = carry;
         }, "add");
  }

  template <typename T, unsigned int bits>
  void negative(bus<T, bits> in,
                bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    bus<T, bits> inv;
    bus<T, bits> one = 1;
    invert(in, inv);
    add(inv, one, out, wire<T>(0), wire<T>());
  }

  template <typename T, unsigned int bits>
  void sub(bus<T, bits> in1,
           bus<T, bits> in2,
           bus<T, bits> out,
           wire<T> borrowin = wire<T>(0),
           wire<T> borrowout = wire<T>())
  {
    static_assert(bits > 0, "bits > 0");
    bus<T, bits> in2inv;
    wire<T> carryin;
    wire<T> carryout;
    invert(in2, in2inv);
    add(in1, in2inv, out, carryin, carryout);
    invert(borrowin, carryin);
    invert(carryout, borrowout);
  }

  template <bool signed_arith = true,
            typename B, typename T, unsigned int bits>
  void compare(bus<T, bits> in1,
               bus<T, bits> in2,
               wire<B> equal,
               wire<B> smaller = wire<B>(),
               wire<B> greater = wire<B>(),
               wire<B> unequal = wire<B>(),
               wire<B> smallerequal = wire<B>(),
               wire<B> greaterequal = wire<B>())
  {
    bus<T, bits> diff;
    wire<T> borrow;
    sub(in1, in2, diff, wire<T>(0), borrow);

    part({ diff, borrow },
         { equal, smaller },
         [=]
         {
           bool all_zero = true;
           for(unsigned int c = 0; c < bits; c++)
             if(diff.at(c) != 0)
               {
                 all_zero = false;
                 break;
               }
           if(all_zero)
             equal = 1;
           else
             equal = 0;

           if(signed_arith)
             smaller = diff.at(bits-1);
           else
             smaller = borrow; // ?
         }, "compare");

    invert(equal, unequal);
    bor(equal, smaller, smallerequal);
    invert(smallerequal, greater);
    invert(smaller, greaterequal);
  }

  template <typename T, unsigned int in_bits, unsigned int out_bits>
  void truncate(bus<T, in_bits> in,
                bus<T, out_bits> out)
  {
    static_assert(in_bits > 0, "in_bits > 0");
    static_assert(out_bits > 0, "out_bits > 0");
    if(out_bits < in_bits)
      {
        part({ in },
             { out },
             [=]
             {
               for(unsigned int c = 0; c < out_bits; c++)
                 out[out_bits-c-1] = in[in_bits-c-1];
             }, "truncate");
      }
    else if(out_bits > in_bits)
      part({ in },
           { out },
           [=]
           {
             for(unsigned int c = 0; c < in_bits; c++)
               out[out_bits-c-1] = in[in_bits-c-1];
             for(unsigned int c = 0; c < out_bits-in_bits; c++)
               out[c] = 0;
           }, "truncate");
    else
      part({ in },
           { out },
           [=]
           {
             for(unsigned int c = 0; c < in_bits; c++)
               out[c] = in[c];
           }, "truncate");
  }    

  template <typename T, unsigned int in_bits, unsigned int out_bits>
  void round(bus<T, in_bits> in,
             bus<T, out_bits> out)
  {
    // TODO: real rounding
    truncate(in, out);
  }     

  template <bool signed_arith = true,
            typename T, unsigned int in_bits, unsigned int out_bits>
  void extend(bus<T, in_bits> in,
              bus<T, out_bits> out)
  {
    static_assert(out_bits >= in_bits, "");
    part({ in },
         { out },
         [=]
         {
           for(unsigned int c = 0; c < in_bits; c++)
             out[c] = in[c];
           for(unsigned int c = 0; c < out_bits-in_bits; c++)
             if(signed_arith)
               out[in_bits+c] = in[in_bits-1];
           else
               out[in_bits+c] = 0;
         }, "extend");
  }

  template <bool signed_arith = true,
            typename T, unsigned int bits>
  void barrel_shift_fixed(bus<T, bits> input,
                          int64_t amount,
                          bus<T, bits> output)
  {
    part({ input },
         { output },
         [=]
         {
           int64_t a = amount;
           if(a == 0)
             output = input;
           else if(a > 0)
             {
               for(unsigned int c = 0; c < a; c++)
                 output[c] = 0;
               for(unsigned int c = 0; c < bits-a; c++)
                 output[a+c] = input[c];
             }
           else if(a < 0)
             {
               a = -a;
               for(unsigned int c = 0; c < bits-a; c++)
                 output[c] = input[a+c];
               for(unsigned int c = 0; c < a; c++)
                 if(signed_arith)
                   output[bits-c-1] = input[bits-1];
                 else
                   output[bits-c-1] = 0;
             }
         }, "barrel_shift_fixed");
  }

  template <bool signed_arith = true,
            typename T, unsigned int bits>
  void barrel_shift(bus<T, bits> input,
                    bus<T, log2ceil(bits)> amount,
                    bus<T, bits> output)
  {
    part({ input, amount },
         { output },
         [=]
         {
           int64_t a = amount;
           if(a == 0)
             output = input;
           else if(a > 0)
             {
               for(unsigned int c = 0; c < a; c++)
                 output[c] = 0;
               for(unsigned int c = 0; c < bits-a; c++)
                 output[a+c] = input[c];
             }
           else if(a < 0)
             {
               for(unsigned int c = 0; c < bits-a; c++)
                 output[c] = input[a+c];
               for(unsigned int c = 0; c < a; c++)
                 if(signed_arith)
                   output[bits-c-1] = input[bits-1];
                 else
                   output[bits-c-1] = 0;
             }
         }, "barrel_shift");
  }

  template <bool signed_arith = true, typename T, unsigned int bits>
  void mul(bus<T, bits> in1,
           bus<T, bits> in2,
           bus<T, 2*bits> out)
  {
    // sign extend
    bus<T, 2*bits> in1_extend;
    extend<signed_arith>(in1, in1_extend);

    // shift
    std::array<bus<T, 2*bits>, bits> tmp;
    for(unsigned int c = 0; c < bits; c++)
      barrel_shift_fixed<signed_arith>(in1_extend, c, tmp[c]);

    // and
    std::array<bus<T, 2*bits>, bits> tmp2;
    std::array<bus<T, 2*bits>, bits> sum;
    for(unsigned int c = 0; c < bits; c++)
      {
        part({ tmp[c], in2[c] },
             { tmp2[c] },
             [=]
             {
               for(unsigned int d = 0; d < 2*bits; d++)
                 tmp2[c][d] = tmp[c][d] & in2[c];
             }, "mul_and");
      }

    // sum
    assign(tmp[0], sum[0]);
    for(unsigned int c = 0; c < bits-1; c++)
      if(c < bits-2 || !signed_arith)
        add(sum[c], tmp2[c+1], sum[c+1]);
      else
        sub(sum[c], tmp2[c+1], sum[c+1]);
    assign(sum[bits-1], out);
  }

  template <typename B, typename T, unsigned int bits>
  void integrator(wire<B> clk,
                  wire<B> reset,
                  wire<B> enable,
                  bus<T, bits> in,
                  bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    bus<T, bits> tmp;
    add(out, in, tmp);
    reg(clk, reset, enable, tmp, out);
  }

  template <typename B, typename T, unsigned int bits>
  void differentiator(wire<B> clk,
                      wire<B> reset,
                      wire<B> enable,
                      bus<T, bits> in,
                      bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    bus<T, bits> tmp;
    reg(clk, reset, enable, in, tmp);
    sub(in, tmp, out);
  }

  template <typename B, typename T, unsigned int bits>
  void counter(wire<B> clk,
               wire<B> reset,
               wire<B> enable,
               bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    bus<T, bits> one = 1;
    integrator(clk, reset, enable, one, out);
  }

  template <bool usep, bool usei, bool used, unsigned int pre_gain, bool signed_arith = true,
            typename B, typename T, unsigned int bits>
  void pidctl(wire<B> clk,
              wire<B> reset,
              wire<B> enable,
              bus<T, bits> input,
              bus<T, log2ceil(bits+pre_gain)> pgain,
              bus<T, log2ceil(bits+pre_gain)> igain,
              bus<T, log2ceil(bits+pre_gain)> dgain,
              bus<T, bits> output)
  {
    static_assert(bits > 1, "bits > 1");

    const unsigned int int_bits = bits+pre_gain;
    
    // pre-gain
    bus<T, int_bits> input2;
    extend(input, input2);

    // proportional
    bus<T, int_bits> input_reg, resultp;
    reg(clk, reset, enable, input2, input_reg);
    barrel_shift(input_reg, pgain, resultp);

    // integral
    bus<T, int_bits> input_int, resulti;
    integrator(clk, reset, enable, input2, input_int);
    barrel_shift(input_int, igain, resulti);

    // differential
    bus<T, int_bits> input_dif, resultd;
    differentiator(clk, reset, enable, input2, input_dif);
    barrel_shift(input_dif, dgain, resultd);

    bus<T, int_bits> sum;
    // add
    if(usep && !usei && !used)
      assign(resultp, sum);
    else if(!usep && usei && !used)
      assign(resulti, sum);
    else if(!usep && !usei && used)
      assign(resultd, sum);
    else if(usep && usei && !used)
      add(resultp, resulti, sum);
    else if(usep && !usei && used)
      add(resultp, resultd, sum);
    else if(!usep && usei && used)
      add(resulti, resultd, sum);
    else if(usep && usei && used)
      {
        bus<T, int_bits> tmp;
        add(resultp, resulti, tmp);
        add(resultd, tmp, sum);
      }
    round(sum, output);
  }

  template <typename T, unsigned int phase_bits, unsigned int bits>
  void sincos(bus<T, phase_bits> phase,
              bus<T, bits> sin,
              bus<T, bits> cos)
  {
    part({ phase },
         { sin, cos },
         [=]
         {
           uint64_t iphase = phase;
           long double dphase = static_cast<long double>(iphase)
             /std::pow(2.l, phase_bits)*2.l*std::acos(-1.l);
           long double dsin = std::sin(dphase);
           long double dcos = std::cos(dphase);
           int64_t isin = dsin*(std::pow(2.l, bits-1)-1);
           int64_t icos = dcos*(std::pow(2.l, bits-1)-1);
           sin = isin;
           cos = icos;
         });
  }

  template <typename B, typename T, unsigned int freq_bits, unsigned int bits>
  void nco(wire<B> clk,
           wire<B> reset,
           wire<B> enable,
           bus<T, freq_bits> freq,
           bus<T, freq_bits> mod,
           bus<T, bits> sine,
           bus<T, bits> cosine,
           bus<T, bits> saw)
  {
    bus<T, freq_bits> phase;
    bus<T, freq_bits> phase2;
    integrator(clk, reset, enable, freq, phase);
    add(phase, mod, phase2);
    sincos(phase2, sine, cosine);
    truncate(phase2, saw);
  }

  template<unsigned int pre_gain,
           typename B, typename T, unsigned int bits, unsigned int freq_bits>
  void pll(wire<B> clk,
           wire<B> reset,
           wire<B> enable,
           bus<T, bits> input,
           bus<T, freq_bits> freq_start,
           bus<T, log2ceil(bits+pre_gain)> pgain,
           bus<T, log2ceil(bits+pre_gain)> igain,
           bus<T, freq_bits> freq_out,
           bus<T, bits> i,
           bus<T, bits> q,
           bus<T, bits> error)
  {
    bus<T, bits> sine;
    bus<T, bits> cosine;
    nco(clk,
        reset,
        enable,
        freq_out,
        bus<T, freq_bits>(0),
        sine,
        cosine,
        bus<T, bits>());

    bus<T, 2*bits> i2;
    bus<T, 2*bits> q2;
    mul<true>(input, sine, i2);
    mul<true>(input, sine, q2);
    truncate(i2, i);
    truncate(q2, q);

    bus<T, bits> pidout;
    bus<T, freq_bits> pidout2;

    pidctl<true, true, false, 16>(clk,
                                  reset,
                                  enable,
                                  error,
                                  pgain,
                                  igain,
                                  bus<T, log2ceil(bits+16)>(0),
                                  pidout);

    truncate(pidout, pidout2);

    add(pidout2, freq_start, freq_out);
  }

  template<unsigned int n, unsigned int r, bool signed_arith = true,
           typename B, typename T, unsigned int in_bits, unsigned int out_bits>
  void cic_down(wire<B> clk,
                wire<B> clk2,
                wire<B> reset,
                wire<B> enable,
                bus<T, in_bits> input,
                bus<T, out_bits> output)
  {
    const unsigned int bits2 = n*r + in_bits;
    std::array<bus<T, bits2>, n+1> ints;
    bus<T, out_bits> tmp;
    std::array<bus<T, out_bits>, n+1> combs;

    extend<signed_arith>(input, ints.at(0));

    for(unsigned int c = 0; c < n; c++)
      integrator(clk,
                 reset,
                 enable,
                 ints.at(c),
                 ints.at(c+1));

    truncate(ints.at(n), tmp);

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

    round(combs.at(n), output);
  }

  template<unsigned int n, unsigned int r, bool signed_arith = true,
           typename B, typename T, unsigned int in_bits, unsigned int out_bits>
  void cic_up(wire<B> clk,
              wire<B> clk2,
              wire<B> reset,
              wire<B> enable,
              bus<T, in_bits> input,
              bus<T, out_bits> output)
  {
    std::array<bus<T, in_bits>, n+1> combs;
    const unsigned int bits2 = n*r + in_bits;
    bus<T, bits2> tmp;
    std::array<bus<T, bits2>, n+1> ints;

    assign(input, combs.at(0));

    for(unsigned int c = 0; c < n; c++)
      differentiator(clk,
                     reset,
                     enable,
                     combs.at(c),
                     combs.at(c+1));

    extend<signed_arith>(combs.at(n), tmp);

    reg(clk2,
        reset,
        enable,
        tmp,
        ints.at(0));

    for(unsigned int c = 0; c < n; c++)
      integrator(clk2,
                 reset,
                 enable,
                 ints.at(c),
                 ints.at(c+1));

    round(ints.at(n), output);
  }

  template<unsigned int bits,
           typename T>
  void pwm(wire<T> clk,
           wire<T> reset,
           wire<T> enable,
           bus<T, log2ceil(bits)> ratio,
           wire<T> output)
  {
    bus<T, log2ceil(bits)> cnt_out;
    wire<T> cnt_rst;

    counter(clk,
            reset,
            enable,
            cnt_out);

    wire<T> equal;
    compare(cnt_out,
            bus<T, log2ceil(bits)>(bits),
            equal);

    wire<T> nreset;
    invert(reset, nreset);
    bnor(equal, nreset, cnt_rst);

    compare<false>(cnt_out, ratio, wire<T>(), output);
  }

  template<unsigned int div,
           typename B>
  void clkdiv(wire<B> clk,
              wire<B> reset,
              wire<B> enable,
              wire<B> clk_out)
  {
    bus<B, log2ceil(div)> ratio(div/2);
    pwm<div>(clk,
             reset,
             enable,
             ratio,
             clk_out);
  }
}

#endif
