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

int main()
{
  const unsigned int bits = 9;
  
  wire<std_logic> clk;
  wire<std_logic> reset;

  wire<fixed_t<true, bits, 0>> one(1), tmp, count;
  wire<fixed_t<true, 2*bits, 0>> square;
  
  add(count, one, tmp);
  reg(clk, reset, wire<std_logic>(1), tmp, count);
  mul(count, count, square);

  print(clk);
  print(reset);
  print(count);
  print(square);

  clk.setname("clk");
  reset.setname("reset");
  one.setname("one");
  tmp.setname("tmp");
  count.setname("count");
  square.setname("square");

  part testbench = part({ },
                        { clk, reset },
                        [=] (uint64_t time)
                        {
                          switch(time % 2)
                            {
                            case 0:
                              clk = 0;
                              break;
                            case 1:
                              clk = 1;
                              break;
                            }

                          if(time < 10)
                            reset = 0;
                          else
                            reset = 1;
                        });

  simulator sim(testbench);
  sim.run(100);
  
  return 0;
}
