#ifndef WIRE_HPP
#define WIRE_HPP

#include <iostream>
#include <memory>
#include <base.hpp>
#include <process.hpp>

namespace hdl
{
  namespace detail
  {
    template <typename T>
    class wire_base : public base
    {
    public:
      T state;
      T prev_state;
      T next_state;
      bool first;
      
      virtual void update()
      {
        prev_state = state;
        state = next_state;
      }
      
      bool changed()
      {
        if(!first)
          return state != next_state;
        first = false;
        return true;
      }
      
      wire_base(std::string name, T initial)
        : base(name), state(initial), prev_state(initial),
          next_state(initial), first(true)
      {
      }
    };
  }

  template <typename T>
  class wire : protected base
  {
    std::shared_ptr<detail::wire_base<T> > w;
    void update() {};
    
  public:
    wire(std::string name, T initial = T())
      : w(new detail::wire_base<T>(name, initial))
    {
      base::wires.push_back(w);
    }

    wire(T initial, std::string name = "wire")
      : w(new detail::wire_base<T>(name, initial))
    {
      base::wires.push_back(w);
    }

    operator T() const
    {
      return w->state;
    }

    void operator=(const T &t) const
    {
      w->next_state = t;
    }

    void operator=(const wire<T> &w2) const
    {
      w->next_state = (T)w2;
    }

    bool event() const
    {
      return w->prev_state != w->state;
    }

    operator std::shared_ptr<base>() const
    {
      return w;
    }
  };

  template <typename T>
  std::ostream& operator<<(std::ostream& lhs, wire<T> const& rhs)
  {
    lhs << (T)rhs;
    return lhs;
  }
}

#endif
