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
      std::function<void(uint64_t)> logic;
      void update(uint64_t time);
      
    public:
      part_int(std::list<std::list<std::shared_ptr<detail::wire_base> > > inputs,
               std::list<std::list<std::shared_ptr<detail::wire_base> > > outputs,
               std::function<void(uint64_t)> logic,
               std::string name);
    };
  }

  class simulator;

  class part
  {
    std::shared_ptr<detail::part_int> p;
    friend class simulator;

  public:
    part(std::list<std::list<std::shared_ptr<detail::wire_base> > > inputs,
         std::list<std::list<std::shared_ptr<detail::wire_base> > > outputs,
         std::function<void(uint64_t)> logic,
         std::string name = "unknown");
  };
}

#endif
