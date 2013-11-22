#include <iostream>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  // logic desription

  wire<int> clk(0, "clk");
  wire<int> reset(0, "reset");
  wire<int> din("din");
  wire<int> dout("dout");
  wire<int> din2("din2");
  wire<int> dout2("dout2");

  reg(clk, reset, wire<int>(1), din, dout, "reg1");
  reg(dout, reset, wire<int>(1), din2, dout2, "reg2");
  
  process
    ({dout, dout2}, {din, din2}, [=]
       {
         din = not dout;
         din2 = not dout2;
       }, "not");

  // testbench

  for(int c = 0; c < 42; c++)
    {
      clk = not clk;
      
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
