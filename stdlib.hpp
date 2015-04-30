#ifndef STDLIB_HPP
#define STDLIB_HPP

#include <array>

#include <wire.hpp>
#include <part.hpp>
#include <std_logic.hpp>

namespace hdl
{

  template <typename T>
  constexpr T power(T base, unsigned int exp)
  {
    return exp == 0 ? 1 : base * power(base, exp-1);
  }

  template <typename T>
  constexpr unsigned int log2ceil(T x, unsigned int i = 0)
  {
    return power(2, i) >= x ? i : log2ceil(x, i+1);
  }

  template <typename T, unsigned int width, typename U>
  std::array<T, width> int2bits(U value)
  {
    static_assert(width > 0, "width > 0");
    static_assert(sizeof(U)*8 >= width, "");
    std::array<T, width> result;
    for(unsigned int c = 0; c < width; c++)
      result[c] = power(2, c) & value ? 1 : 0;
    return result;
  }

  template <typename U, typename T, unsigned int width>
  U bits2int(std::array<T, width> data)
  {
    static_assert(width > 0, "width > 0");
    U result = 0;
    for(unsigned int c = 0; c < width; c++)
      result |= data[c] ? power(2, c) : 0;
    return result;
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
           if(reset == 0)
             dout = 0;
           else if(clk.event() and clk == 1 and enable == 1)
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
    for(unsigned int c = 0; c < bits; c++)
      invert(in[c], out[c]);
  }

  template <typename T, unsigned int bits>
  void add(bus<T, bits> in1,
           bus<T, bits> in2,
           bus<T, bits> out,
           wire<T> carryin = wire<T>(0),
           wire<T> carryout = wire<T>(0))
  {
    static_assert(bits > 0, "bits > 0");
    part({ in1, in2 },
         { out },
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
    bus<T, bits> one = int2bits<T, bits>(1);
    invert(in, inv);
    add(inv, one, out, wire<T>(0), wire<T>());
  }

  template <typename T, unsigned int bits>
  void sub(bus<T, bits> in1,
           bus<T, bits> in2,
           bus<T, bits> out,
           wire<T> borrowin,
           wire<T> borrowout)
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
    add(out, in, tmp, wire<T>(0), wire<T>(0));
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
    reg(clk, reset, enable, tmp, out);
    sub(out, in, tmp, wire<T>(0), wire<T>(0));
  }
    
  template <typename B, typename T, unsigned int bits>
  void counter(wire<B> clk,
               wire<B> reset,
               wire<B> enable,
               bus<T, bits> out)
  {
    static_assert(bits > 0, "bits > 0");
    bus<T, bits> one = int2bits<T, bits>(1);
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
             out = in;
           }, "truncate");
  }    

  template <typename T, unsigned int in_bits, unsigned int out_bits>
  void round(bus<T, in_bits> in,
             bus<T, out_bits> out)
  {
    // TODO: real rounding
    truncate(in, out);
  }     

  template <typename T, unsigned int in_bits, unsigned int out_bits, bool signed_arith = true>
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
             out[in_bits+c] = signed_arith ? in[in_bits-1] : 0;
         }, "extend");
  }

  template <typename T, unsigned int bits, bool signed_arith = true>
  void barrel_shift(bus<T, bits> input,
                    bus<T, log2ceil(bits)> amount,
                    bus<T, bits> output)
  {
    part({ input, amount },
         { output },
         [=]
         {
           int64_t a = bits2int<int64_t>(amount);
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
                 output[bits-c-1] = signed_arith ? input[bits-1] : 0;
             }
         }, "barrel_shift");
  }

  template <bool usep, bool usei, bool used, bool signed_arith, bool gains_first,
            typename B, typename T, unsigned int bits, unsigned int int_bits>
  void pidctl(wire<B> clk,
              wire<B> reset,
              wire<B> enable,
              bus<T, bits> input,
              bus<T, log2ceil(int_bits)> pgain,
              bus<T, log2ceil(int_bits)> igain,
              bus<T, log2ceil(int_bits)> dgain,
              bus<T, bits> output)
  {
    static_assert(bits > 1, "bits > 1");
    static_assert(int_bits >= bits, "int_bits >= bits");
    
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
      add(resulti, resultd);
    else if(usep && usei && used)
      {
        bus<T, int_bits> tmp;
        add(resultp, resulti, tmp);
        ass(resultd, tmp, sum);
      }
    
  }
  
}

#endif
