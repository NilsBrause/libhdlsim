#include <iostream>
#include <tuple>

#include <hdl.hpp>

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
