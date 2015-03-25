#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  wire<std_logic> clk("clk", 0);
  wire<std_logic> reset("reset", 0);
  wire<std_logic> enable("enable", 1);
  wire<int> out("out", 0);
  
  counter(clk, reset, enable, out);

  for(unsigned int c = 0; c < 20; c++)
    {
      clk = !clk;
      if(c < 10)
        reset = 0;
      else
        reset = 1;
      waitfor(1);
      if(clk == 1)
        std::cout << out << std::endl;
    }
  
  return 0;
}
