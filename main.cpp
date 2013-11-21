#include <iostream>

#include <hdl.hpp>
//#include <std_logic.hpp>

using namespace hdl;

#define std_logic bool

int main()
{
  // logic desription

  auto clk = wire<std_logic>::create("clk");
  auto reset = wire<std_logic>::create("reset");
  auto enable = wire<std_logic>::create("enable");
  auto din = wire<std_logic>::create("din");
  auto dout = wire<std_logic>::create("dout");
  auto din2 = wire<std_logic>::create("din2");
  auto dout2 = wire<std_logic>::create("dout2");
  
  reg(clk, reset, enable, din, dout, "reg1");
  reg(dout, reset, enable, din2, dout2, "reg2");

  din << not dout;
  din2 << not dout2;

  // initialize
  clk->set(0);
  reset->set(0);
  enable->set(1);
  base::waitfor(1);

  // testbench
  for(int c = 0; c < 42; c++)
    {
      clk->set(not clk->get());
      
      if(base::waitfor() <= 10)
        reset->set(0);
      else
        reset->set(1);
      
      base::waitfor(1);
      
      std::cout << "(" << clk->get() << " | " << reset->get() << ") "
                << din->get() << " " << dout->get() << " | "
                << din2->get() << " " << dout2->get() << std::endl;
    }

  return 0;
}
