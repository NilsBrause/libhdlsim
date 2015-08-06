#ifndef PART_HPP
#define PART_HPP

#include <functional>
#include <memory>
#include <base.hpp>

namespace hdl
{
  namespace detail
  {
    class part_int : public base
    {
      std::function<void(uint64_t)> logic;
      virtual void update(uint64_t time);
      virtual bool changed();
      
    public:
      part_int(std::list<std::list<std::shared_ptr<detail::base> > > inputs,
               std::list<std::list<std::shared_ptr<detail::base> > > outputs,
               std::function<void(uint64_t)> logic);
    };
  }

  class simulator;

  class part
  {
    std::shared_ptr<detail::part_int> p;
    friend class simulator;

  public:
    part(std::list<std::list<std::shared_ptr<detail::base> > > inputs,
         std::list<std::list<std::shared_ptr<detail::base> > > outputs,
         std::function<void(uint64_t)> logic,
         std::string name = "unknown");
  };
}

#endif
