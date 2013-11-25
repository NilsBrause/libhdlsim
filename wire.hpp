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
    class wire_int : public wire_base
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

      void set(const T &t)
      {
        lock();
        next_state = t;
        unlock();
      }
      
      T get()
      {
        lock();
        T t = state;
        unlock();
        return t;
      }

      bool event()
      {
        lock();
        bool b = prev_state != state;
        unlock();
        return b;
      }
      
      wire_int(std::string name, T initial)
        : wire_base(name), state(initial), prev_state(initial),
          next_state(initial), first(true)
      {
        omp_init_nest_lock(&omp_lock);
      }
    };
  }

  template <typename T>
  class wire
  {
    std::shared_ptr<detail::wire_int<T> > w;

  public:
    wire(std::string name, T initial = T())
      : w(new detail::wire_int<T>(name, initial))
    {
      detail::wires.push_back(w);
    }

    wire(T initial = T(), std::string name = "wire")
      : w(new detail::wire_int<T>(name, initial))
    {
      detail::wires.push_back(w);
    }

    operator T() const
    {
      return w->state;
    }

    void operator=(const T &t) const
    {
      w->set(t);
    }

    void operator=(const wire<T> &w2) const
    {
      w->set((T)w2);
    }

    bool event() const
    {
      return w->event();
    }

    operator std::shared_ptr<detail::wire_base>() const
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
