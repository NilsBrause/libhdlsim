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
    return detail::power(2u, i) >= x ? i : log2ceil(x, i+1);
  }

  //---------------------------------------------------------------------------

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
  
  template <typename T, unsigned int bits>
  void invert(bus<T, bits> in,
              bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
#if 0
    part({ in },
         { out },
         [=]
         {
           for(unsigned int c = 0; c < bits; c++)
             out[c] = !in[c];
         }, "invert");
#else
    for(unsigned int c = 0; c < bits; c++)
      invert(in[c], out[c]);
#endif
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
           wire<T> borrowout = wire<T>(0))
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
}

#endif
