#ifndef STDLIB_HPP
#define STDLIB_HPP

#include <wire.hpp>
#include <process.hpp>

namespace hdl
{
  template <typename T, typename U, typename V, typename W>
  void reg(wire<U>& clk, wire<V>& reset,
           wire<W>& enable, wire<T>& din, wire<T>& dout,
           std::string name = "reg")
  {
    new process
      ({&clk, &reset}, {&dout}, [&]
       {
         if(!(V)reset)
           dout = T(0);
         else if(clk.rising_edge())
           if((W)enable)
             dout = (T)din;
       }, name);
  }
}

#endif
