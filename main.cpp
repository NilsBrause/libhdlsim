#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  // logic desription

  wire<std_logic> clk("clk", '0');
  wire<std_logic> reset("reset");
  wire<std_logic> din("din");
  wire<std_logic> dout("dout");
  wire<std_logic> din2("din2");
  wire<std_logic> dout2("dout2");
  //wire<int> cnt("cnt");
  wire<std_logic> inout("inout");

  reg("reg1", clk, reset, wire<std_logic>('1'), din, dout);
  reg("reg2", dout, reset, wire<std_logic>('1'), din2, dout2);

  //counter("cnt1", clk, reset, wire<std_logic>("enable", '1'), cnt);
  
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
        reset = '0';
      else
        reset = '1';
      
      waitfor(1);
    }

  return 0;
}
