#include <iostream>
#include <tuple>

#include <hdl.hpp>

using namespace hdl;

int main()
{
  const unsigned int freq_bits = 16;
  const unsigned int bits = 16;
  const unsigned int pre_gain = 16;
  const unsigned int n = 2;
  const unsigned int r = 8;
  wire<std_logic> clk(1);
  wire<std_logic> reset(0);
  bus<std_logic, freq_bits> freq(1000);
  bus<std_logic, freq_bits> freq_start(1000);
  bus<std_logic, freq_bits> freq_out;
  bus<std_logic, freq_bits+n*r> freq_out_slow;
  bus<std_logic, freq_bits> freq_out_inter;
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

  wire<std_logic> clk2;

  clkdiv<power(2, r)>(clk,
            reset,
            wire<std_logic>(1),
            clk2);

  cic_down<n, r>(clk,
                 clk2,
                 reset,
                 wire<std_logic>(1),
                 freq_out,
                 freq_out_slow);

  cic_up<n, r>(clk2,
               clk,
               reset,
               wire<std_logic>(1),
               freq_out_slow,
               freq_out_inter);

  unsigned int f = 2000;
  for(unsigned int c = 0; c < 1000; c++)
    {
      clk = 0;
      waitfor(1);
      clk = 1;

      if(c < 10)
        reset = 0;
      else
        reset = 1;

      f++;
      freq = f;

      waitfor(1);

      std::cout << c << " "
                << static_cast<int64_t>(freq) << " "
                << static_cast<int64_t>(freq_out) << " "
                << static_cast<int64_t>(freq_out_slow)/power(2, n*r) << " "
                << static_cast<int64_t>(freq_out_inter) << " "
                << std::endl;
    }
  
  return 0;
}
