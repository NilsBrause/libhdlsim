#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <functional>
#include <base.hpp>

namespace hdl
{
  class process : public base
  {
    std::function<void()> logic;
    void update();
    
  public:
    process(std::list<base*> sensitivitylist,
            std::list<base*> outputs,
            std::function<void()> logic,
            std::string name = "process");
  };
}

#endif
