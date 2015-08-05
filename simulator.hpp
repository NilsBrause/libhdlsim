#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <part.hpp>

namespace hdl
{
  class simulator
  {
  private:
    part tb;
    uint64_t cur_time;

  public:
    simulator(part testbench);
    void run(uint64_t duration);
  };
}

#endif
