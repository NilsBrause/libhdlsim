#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  wire<std_logic> clk(1);
  wire<std_logic> reset(0);
  const unsigned int bits = 32;
  const unsigned int pre_gain = 8;
  bus<std_logic, bits> input = 42;
  bus<std_logic, bits> output;

  pidctl<true, true, true, pre_gain>(clk,
                                     reset,
                                     wire<std_logic>(1),
                                     input,
                                     bus<std_logic, log2ceil(bits+pre_gain)>(1),
                                     bus<std_logic, log2ceil(bits+pre_gain)>(1),
                                     bus<std_logic, log2ceil(bits+pre_gain)>(1),
                                     output);
  
  for(unsigned int c = 0; c < 100000; c++)
    {
      clk = 0;
      waitfor(1);
      clk = 1;
      if(c < 10)
        reset = 0;
      else
        reset = 1;
      waitfor(1);
      std::cout << output << std::endl;
    }
  
  return 0;
}
