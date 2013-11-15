#ifndef WIRE_HPP
#define WIRE_HPP

#include <iostream>
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

    friend  std::ostream& operator<<<T>(std::ostream& os, const wire<T>& rhs);

    virtual void update()
    {
#ifdef DEBUG
      std::cout << "wire " << myname << " has been updated" << std::endl;
#endif
      prev_state = state;
      state = next_state;
    }

    bool changed()
    {
      return state != next_state;
    }

  public:
    wire(std::string name = "wire")
      : base(name)
    {
      wires.push_back(this);
    }

    ~wire()
    {
      wires.remove(this);
    }

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

    operator T()
    {
      return state;
    }

    wire<T>& operator=(const T& rhs)
    {
      set(rhs);
      return *this;
    }

    bool operator==(const T& rhs)
    {
      return get() == rhs;
    }

    bool operator!=(const T& rhs)
    {
      return get() != rhs;
    }

    ///////////////////////////////////////////////////////////////////////////

    wire<T>& operator=(wire<T>& rhs)
    {
      wire<T> *lhs = this;
      new process
        ({&rhs}, {lhs}, [=, &rhs]
         {
           lhs->set(rhs.get());
         });
      return *this;
    }

    wire<T> &operator not()
    {
      wire *lhs = new wire<T>();
      wire *rhs = this;
      new process
        ({rhs}, {lhs}, [=]
         {
           lhs->set(not rhs->get());
         });
      return *lhs;
    }

    wire<bool> &operator==(wire<T>& rhs)
    {
      wire<bool> *w = new wire<bool>();
      wire<T> *lhs = this;
      new process
        ({lhs, &rhs}, {w}, [=, &rhs]
         {
           w->set(lhs->get() == rhs.get());
         });
      return *w;
    }

    bool operator!=(wire<T>& rhs)
    {
      wire<bool> *w = new wire<bool>();
      wire<T> *lhs = this;
      new process
        ({lhs, &rhs}, {w}, [=, &rhs]
         {
           w->set(lhs->get() != rhs.get());
         });
      return *w;
    }
  };

  template <typename T>
  std::ostream &operator<<(std::ostream& os, const wire<T>& rhs)
  {
    os << rhs.state;
    return os;
  }
}

#endif
