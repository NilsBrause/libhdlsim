#ifndef STDLIB_HPP
#define STDLIB_HPP

#include <wire.hpp>
#include <part.hpp>

namespace hdl
{
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
             dout = T();
           else if(clk.event() and clk == 1 and enable == 1)
             dout = din;
         }, "reg");
  };

  template <typename T>
  void adder(wire<T> in1,
             wire<T> in2,
             wire<T> out)
  {
    part({ in1, in2 },
         { out },
         [=]
         {
           out = in1 + in2;
         }, "adder");
  }

  template <typename B, typename T>
  void counter(wire<B> clk,
               wire<B> reset,
               wire<B> enable,
               wire<T> out)
  {
    wire<T> one("one");
    wire<T> tmp("tmp");
    one = 1;
    reg(clk, reset, enable, tmp, out);
    adder(out, one, tmp);
  }

  template <typename B, typename T>
  void integrator(wire<B> clk,
                  wire<B> reset,
                  wire<B> enable,
                  wire<T> in,
                  wire<T> out)
  {
    wire<T> tmp("tmp");
    reg(clk, reset, enable, tmp, out);
    adder(out, in, tmp);
  }
}

#endif
