#ifndef STDLIB_HPP
#define STDLIB_HPP

#include <wire.hpp>
#include <process.hpp>

namespace hdl
{
  template <typename T, typename U, typename V, typename W>
  void reg(std::string name, wire<T> clk, wire<U> reset,
           wire<V> enable, wire<W> din, wire<W> dout)
  {
    process
      (name, {clk, reset}, {dout}, [=]
       {
         if(reset == 0)
           dout = 0;
         else if(clk.event() and clk == 1)
           if(enable == 1)
             dout = din;
       });
  }

  template <typename T>
  void adder(std::string name, wire<T> input1,
             wire<T> input2, wire<T> output)
  {
    process
      ("adder", {input1, input2}, {output}, [=]
       {
         output = input1 + input2;
       });
  }

  template <typename T, typename U, typename V, typename W>
  void integrator(std::string name, wire<T> clk, wire<U> reset,
                  wire<V> enable, wire<W> input, wire<W> output)
  {
    wire<int> tmp;
    reg("reg", clk, reset, enable, tmp, output);
    adder("add", output, input, tmp);
  }

  template <int direction = 1, typename T, typename U, typename V, typename W>
  void counter(std::string name, wire<T> clk, wire<U> reset,
               wire<V> enable, wire<W> output)
  {
    integrator("int", clk, reset, enable, direction, output);
  }
}

#endif
