#ifndef WIRE_HPP
#define WIRE_HPP

#include <iostream>
#include <memory>
#include <base.hpp>
#include <process.hpp>

namespace hdl
{
  template <typename T>
  class wire;

  template <typename T>
  std::ostream& operator<<(std::ostream& os, const wire<T>& rhs);

  template <typename T>
  class wire : public base
  {
  private:
    T state;
    T prev_state;
    T next_state;
    bool first;

    friend  std::ostream& operator<<<T>(std::ostream& os, const wire<T>& rhs);

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

    wire(std::string name = "wire")
      : base(name), first(true)
    {
    }

  public:
    T get()
    {
      return state;
    }

    void set(T t)
    {
      next_state = t;
    }

    bool event()
    {
      return prev_state != state;
    }
      
    bool rising_edge()
    {
      return get() && event();
    }
    
    bool falling_edge()
    {
      return !get() && event();
    }

    static std::shared_ptr<wire<T> > create(std::string name)
    {
      std::shared_ptr<wire<T> > w(new wire<T>(name));
      base::wires.push_back(w);
      return w;
    }
  };

  template <typename T>
  std::ostream &operator<<(std::ostream& os, const std::shared_ptr<wire<T> > rhs)
  {
    os << rhs.state;
    return os;
  }

  template<typename T>
  std::shared_ptr<wire<T> > operator<<(std::shared_ptr<wire<T> > lhs, std::shared_ptr<wire<T> > rhs)
  {
    process::create
      ({rhs}, {lhs}, [=]
       {
         lhs->set(rhs->get());
       }, "assign");
    return lhs;
  }

  template<typename T>
  std::shared_ptr<wire<T> > operator not(std::shared_ptr<wire<T> > rhs)
  {
    auto lhs = wire<T>::create("tmp");
    process::create
      ({rhs}, {lhs}, [=]
       {
         lhs->set(not rhs->get());
       }, "not");
    return lhs;
  }
}

#endif
