#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  const unsigned int bits = 9;
  
  wire<std_logic> clk(1);
  wire<std_logic> reset(0);
  
  bus<std_logic, bits> one = 1;
  bus<std_logic, bits> tmp;
  bus<std_logic, bits> count;
  add(count, one, tmp);
  reg(clk, reset, wire<std_logic>(1), tmp, count);

  for(unsigned int c = 0; c < 100; c++)
    {
      clk = 0;
      waitfor(1);
      clk = 1;

      if(c < 10)
        reset = 0;
      else
        reset = 1;

      waitfor(1);

      std::cout << clk << " " << reset << " "
                << static_cast<int16_t>(count) << " "
                << std::endl;
    }
  
  return 0;
}
