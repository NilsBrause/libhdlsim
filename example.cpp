#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  const unsigned int bits = 9;
  
  wire<std_logic> clk;
  wire<std_logic> reset;
  
  bus<std_logic, bits> one = 1;
  bus<std_logic, bits> tmp;
  bus<std_logic, bits> count;
  add(count, one, tmp);
  reg(clk, reset, wire<std_logic>(1), tmp, count);

  print(clk);
  print(reset);
  print(count);

  clk.setname("clk");
  reset.setname("reset");
  one.setname("one");
  tmp.setname("tmp");
  count.setname("count");

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
