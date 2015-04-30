#ifndef PART_HPP
#define PART_HPP

#include <functional>
#include <memory>
#include <base.hpp>

namespace hdl
{
  namespace detail
  {
    class part_int : public part_base
    {
      std::function<void()> logic;
      void update();
      
    public:
      part_int(std::list<std::list<std::shared_ptr<detail::wire_base> > > inputs,
               std::list<std::list<std::shared_ptr<detail::wire_base> > > outputs,
               std::function<void()> logic,
               std::string name);
    };
  }

  class part
  {
    std::shared_ptr<detail::part_int> p;

  public:
    part(std::list<std::list<std::shared_ptr<detail::wire_base> > > inputs,
         std::list<std::list<std::shared_ptr<detail::wire_base> > > outputs,
         std::function<void()> logic,
         std::string name = "unknown");
  };
}

#endif
