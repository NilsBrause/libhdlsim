#include <iostream>
#include <hdl.hpp>

using namespace hdl;

int main()
{
  // logic desription

  wire<bool> clk("clk");
  wire<bool> reset("reset");
  wire<bool> enable("enable");
  wire<bool> din("din");
  wire<bool> dout("dout");
  wire<bool> din2("din2");
  wire<bool> dout2("dout2");

  reg<bool>(clk, reset, enable, din, dout, "reg1");
  reg<bool>(dout, reset, enable, din2, dout2, "reg2");

  din = not dout;
  din2 = not dout2;

  // initialize
  clk = 0;
  reset = 0;
  enable = 1;
  base::waitfor(1);

  // testbench
  for(int c = 0; c < 42; c++)
    {
      clk = not (bool)clk;
      
      if(base::waitfor() <= 10)
        reset = 0;
      else
        reset = 1;
      
      base::waitfor(1);
      
      std::cout << "(" << clk << " | " << reset << ") "
                << din << " " << dout << " | "
                << din2 << " " << dout2 << std::endl;
    }

  return 0;
}
