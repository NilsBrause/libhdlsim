#ifndef STDLIB_HPP
#define STDLIB_HPP

#include <wire.hpp>
#include <process.hpp>

namespace hdl
{
  template <typename T, typename U, typename V, typename W>
  void reg(wire<U> clk, wire<V> reset, wire<W> enable,
           wire<T> din, wire<T> dout, std::string name)
  {
    process
      ({clk, reset}, {dout}, [=]
       {
         if(reset == 0)
           dout = 0;
         else if(clk.event() and clk == 1)
           if(enable == 1)
             dout = din;
       }, name);
  }
}

#endif
