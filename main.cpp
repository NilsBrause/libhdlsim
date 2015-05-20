#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  const unsigned int freq_bits = 16;
  const unsigned int bits = 16;
  const unsigned int pre_gain = 16;
  wire<std_logic> clk(1);
  wire<std_logic> reset(0);
  bus<std_logic, freq_bits> freq(1300);
  bus<std_logic, freq_bits> freq_start(1000);
  bus<std_logic, freq_bits> freq_out;
  bus<std_logic, bits> sine;
  bus<std_logic, bits> i;
  bus<std_logic, bits> q;

  nco(clk,
      reset,
      wire<std_logic>(1),
      freq,
      bus<std_logic, freq_bits>(0),
      sine,
      bus<std_logic, bits>(),
      bus<std_logic, bits>());

  pll<pre_gain>(clk,
      reset,
      wire<std_logic>(1),
      sine,
      freq_start,
      bus<std_logic, log2ceil(bits+pre_gain)>(12),
      bus<std_logic, log2ceil(bits+pre_gain)>(6),
      freq_out,
      i,
      q,
      q);

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
                << static_cast<int32_t>(freq) << " "
                << static_cast<int32_t>(freq_out) << " "
                << static_cast<int32_t>(q) << " "
                << std::endl;
    }
  
  return 0;
}
