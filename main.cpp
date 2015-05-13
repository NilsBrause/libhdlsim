#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  wire<std_logic> clk(1);
  wire<std_logic> reset(0);
  const unsigned int freq_bits = 16;
  const unsigned int bits = 16;
  bus<std_logic, freq_bits> freq(1024);
  bus<std_logic, bits> sine;

  nco(clk, reset, wire<std_logic>(1), freq, bus<std_logic, freq_bits>(0), sine, bus<std_logic, bits>(), bus<std_logic, bits>());
  
  for(unsigned int c = 0; c < 10000; c++)
    {
      clk = 0;
      waitfor(1);
      clk = 1;
      if(c < 10)
        reset = 0;
      else
        reset = 1;
      waitfor(1);
      std::cout << c << " " << (int32_t)sine << std::endl;
    }
  
  return 0;
}
