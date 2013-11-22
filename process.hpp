#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <functional>
#include <memory>
#include <base.hpp>

namespace hdl
{
  namespace detail
  {
    class process_base : public base
    {
      std::function<void()> logic;
      void update();
      
    public:
      process_base(std::list<std::shared_ptr<base> > outputs,
                   std::function<void()> logic, std::string name);
    };
  }

  class process : protected base
  {
    std::shared_ptr<detail::process_base> p;
    void update() {};

  public:
    process(std::list<std::shared_ptr<base> > sensitivity_list,
            std::list<std::shared_ptr<base> > outputs,
            std::function<void()> logic, std::string name);
  };
}

#endif
