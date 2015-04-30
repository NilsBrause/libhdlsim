#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  wire<std_logic> clk(1);
  wire<std_logic> reset(0);
  const unsigned int bits = 32;
  bus<std_logic, bits> out;

  counter(clk, reset, wire<std_logic>(1), out);
  
  for(unsigned int c = 0; c < 1000000; c++)
    {
      clk = 0;
      waitfor(1);
      clk = 1;
      if(c < 10)
        reset = 0;
      else
        reset = 1;
      waitfor(1);
      if(reset == 1 && clk == 1)
        std::cout << out << std::endl;
    }
  
  return 0;
}
