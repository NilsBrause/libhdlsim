#include <iostream>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  // logic desription

  wire<int> clk("clk", 0);
  wire<int> reset("reset", 0);
  wire<int> din("din", 0);
  wire<int> dout("dout", 0);
  wire<int> din2("din2", 0);
  wire<int> dout2("dout2", 0);
  wire<int> cnt("cnt", 0);
  wire<std_logic> inout("inout", 'U');

  reg("reg1", clk, reset, wire<int>(1), din, dout);
  reg("reg2", dout, reset, wire<int>(1), din2, dout2);

  counter("cnt1", clk, reset, wire<int>(1), cnt);
  
  process
    ("notnot", {dout, dout2}, {din, din2}, [=]
       {
         din = not dout;
         din2 = not dout2;
       });

  process
    ("test1", {reset}, {inout}, [=]
     {
       if(reset == 0)
         inout = '0';
       else
         inout = 'Z';
     });

  process
    ("test2", {reset}, {inout}, [=]
     {
       if(reset == 0)
         inout = 'Z';
       else
         inout = '1';
     });

  // testbench

  for(int c = 0; c < 42; c++)
    {
      clk = not clk;
      
      if(waitfor() < 10)
        reset = 0;
      else
        reset = 1;
      
      waitfor(1);
      
      std::cout << "(" << clk << " | " << reset << ") "
                << din << " " << dout << " | "
                << din2 << " " << dout2 << " "
                << cnt << " " << inout << std::endl;
    }

  return 0;
}
