#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <functional>
#include <memory>
#include <base.hpp>

namespace hdl
{
  class process : public base
  {
    std::function<void()> logic;
    void update();
    
    process(std::list<std::shared_ptr<base> > outputs,
            std::function<void()> logic, std::string name);

  public:
    static std::shared_ptr<process>
    create(std::list<std::shared_ptr<base> > sensitivity_list,
           std::list<std::shared_ptr<base> > outputs,
           std::function<void()> logic, std::string name);
  };
}

#endif
