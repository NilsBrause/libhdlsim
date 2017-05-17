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

#include <iostream>
#include <tuple>

#define SYMMETRIC
#include <hdlsim.hpp>

using namespace hdl;

template<unsigned int bits = 9>
class example_t
{
private:
  // declare signals
  wire<std_logic> clk;
  wire<std_logic> reset;
  wire<fixed_t<true, bits, 0>> one, tmp, count;
  wire<fixed_t<true, 2*bits, 0>> square;

  // implement testbench
  part testbench;
  void tb_func(uint64_t time)
  {
    // create clock 
    switch(time % 2)
      {
      case 0:
        clk = 0;
        break;
      case 1:
        clk = 1;
        break;
      }

    // create active low reset
    if(time < 10)
      reset = 0;
    else
      reset = 1;
  }

public:
  example_t()
  {
    // set upinitial values
    one = 1;

    // connect components
    add(count, one, tmp);
    reg(clk, reset, wire<std_logic>(1), tmp, count);
    mul(count, count, square);

    // give names to interesting signals
    clk.setname("clk");
    reset.setname("reset");
    count.setname("count");
    square.setname("square");

    // print interesting signals
    print(clk);
    print(reset);
    print(count);
    print(square);

    // create testbench part
    testbench = part({ }, { clk, reset }, [this] (uint64_t time) { this->tb_func(time); });
  }

  void run(unsigned int duration)
  {
    // create and run simulation
    simulator sim(testbench);
    sim.run(duration);
  }
};

int main()
{
  example_t<> example;
  example.run(100);
  return 0;
}
