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
  bus<std_logic, 2*bits> sine2;

  bus<std_logic, 4> x(7);
  bus<std_logic, 4> y(3);
  bus<std_logic, 8> z;

  nco(clk,
      reset,
      wire<std_logic>(1),
      freq,
      bus<std_logic, freq_bits>(0),
      sine,
      bus<std_logic, bits>(),
      bus<std_logic, bits>());

  mul<true>(sine, sine, sine2);

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
      std::cout << c << " "
                << static_cast<int32_t>(sine) << " "
                << static_cast<int32_t>(sine2)/32768 << std::endl;
    }
  
  return 0;
}
