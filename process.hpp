#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <functional>
#include <memory>
#include <base.hpp>

namespace hdl
{
  namespace detail
  {
    class process_int : public process_base
    {
      std::function<void()> logic;
      void update();
      
    public:
      process_int(std::string name,
                   std::list<std::shared_ptr<detail::wire_base> > sensitivity_list,
                   std::list<std::shared_ptr<detail::wire_base> > outputs,
                   std::function<void()> logic);
    };
  }

  class process
  {
    std::shared_ptr<detail::process_int> p;

  public:
    process(std::string name,
            std::list<std::shared_ptr<detail::wire_base> > sensitivity_list,
            std::list<std::shared_ptr<detail::wire_base> > outputs,
            std::function<void()> logic);
  };
}

#endif
