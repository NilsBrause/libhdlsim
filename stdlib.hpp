#ifndef STDLIB_HPP
#define STDLIB_HPP

#include <wire.hpp>
#include <process.hpp>

namespace hdl
{
  template <typename T, typename U, typename V, typename W>
  void reg(std::shared_ptr<wire<U> > clk, std::shared_ptr<wire<V> > reset,
           std::shared_ptr<wire<W> > enable, std::shared_ptr<wire<T> > din, std::shared_ptr<wire<T> > dout,
           std::string name = "reg")
  {
    process::create
      ({clk, reset}, {dout}, [=]
       {
         if(!reset->get())
           dout->set(T(0));
         else if(clk->rising_edge())
           if(enable->get())
             dout->set(din->get());
       }, name);
  }
}

#endif
