#ifndef WIRE_HPP
#define WIRE_HPP

#include <iostream>
#include <map>
#include <memory>
#include <base.hpp>
#include <process.hpp>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace hdl
{
  // Resolve multiple assignments to a wire.
  // Has to be reimplemented by types with high-Z support.
  template <typename T>
  T resolve(std::map<hdl::detail::process_base*, T> candidates,
            hdl::detail::wire_base *w)
  {
    std::cerr << "WARNING: wire " << w->getname()
              << " has been updated by the following processes: ";
    for(auto &i : candidates)
      std::cerr << i.first->getname() << " ";
    std::cerr << std::endl;

    return candidates.begin()->second;
  }

  template<typename T>
  class wire;

  namespace detail
  {
    template <typename T>
    class wire_int : public wire_base
    {
    private:
      T state;
      T prev_state;
      std::map<process_base*, T> next_state;
      bool first;

      virtual void update()
      {
        switch(next_state.size())
          {
          case 0:
            break;
          case 1:
            prev_state = state;
            state = next_state.begin()->second;
            break;
          default:
            prev_state = state;
            state = resolve(next_state, this);
            break;
          }
      }
      
      bool changed()
      {
        bool chg = false;
        if(!first)
          {
            for(auto &i : next_state)
              if(state != i.second)
                {
                  chg = true;
                  break;
                }
          }
        else
          {
            chg = true;
            first = false;
          }
        return chg;
      }

      void set(const T &t)
      {
        lock();
#ifdef _OPENMP
        unsigned int n = omp_get_thread_num();
#else
        unsigned int n = 0;
#endif
        process_base *p;
        if(cur_parent.size() == 0) // set from top level testbench
          p = NULL;
        else
          p = cur_parent.at(n);
        next_state[p] = t;
        unlock();
      }
      
      T get()
      {
        return state;
      }

      bool event()
      {
        return prev_state != state;
      }
      
      wire_int(std::string name, T initial)
        : wire_base(name), state(initial), prev_state(initial), first(true)
      {
      }
      
      friend class wire<T>;
    };
  }

  template <typename T>
  class wire
  {
    std::shared_ptr<detail::wire_int<T> > w;

  public:
    wire(T initial = T())
      : w(new detail::wire_int<T>("wire", initial))
    {
      detail::wires.push_back(w);
    }

    wire(std::string name, T initial)
      : w(new detail::wire_int<T>(name, initial))
    {
      detail::wires.push_back(w);
    }

    operator T() const
    {
      return w->get();
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

    // unary operators

#define OPERATOR1(OP)     \
    T operator OP() const \
    {                     \
      return OP w->get(); \
    }                     \

    OPERATOR1(not)
    OPERATOR1(~)
    OPERATOR1(+)
    OPERATOR1(-)

    // binary operators

#define OPERATOR2(RET, OP)                   \
    RET operator OP(const wire<T> &w2) const \
    {                                        \
      return w->get() OP (T)w2;              \
    }                                        \
                                             \
    RET operator OP(const T &t) const        \
    {                                        \
      return w->get() OP t;                  \
    }

    OPERATOR2(bool, ==)
    OPERATOR2(bool, !=)
    OPERATOR2(bool, >)
    OPERATOR2(bool, <)
    OPERATOR2(bool, >=)
    OPERATOR2(bool, <=)
    OPERATOR2(T, &)
    OPERATOR2(T, |)
    OPERATOR2(T, ^)
    OPERATOR2(T, +)
    OPERATOR2(T, -)
    OPERATOR2(T, *)
    OPERATOR2(T, /)
    OPERATOR2(T, %)

    // modifying binary operators

#define OPERATOR3(OP)                             \
    wire<T> &operator OP(const wire<T> &w2) const \
    {                                             \
      w->set(w->get() OP (T)w2);                  \
      return *this;                               \
    }                                             \
                                                  \
    wire<T> &operator OP(const T &t) const        \
    {                                             \
      w->set(w->get() OP t);                      \
      return *this;                               \
    }

    OPERATOR3(&=);
    OPERATOR3(|=);
    OPERATOR3(^=);
    OPERATOR3(+=);
    OPERATOR3(-=);
    OPERATOR3(*=);
    OPERATOR3(/=);
    OPERATOR3(%=);

    // two-type binary operators

#define OPERATOR4(RET, OP)                   \
    template <typename U>                    \
    RET operator OP(const wire<U> &w2) const \
    {                                        \
      return w->get() OP (U)w2;              \
    }                                        \
                                             \
    template <typename U>                    \
    RET operator OP(const U &t) const        \
    {                                        \
      return w->get() OP (U)t;               \
    }

    OPERATOR4(T, <<);
    OPERATOR4(T, >>);

    // modifying two-type binary operators

#define OPERATOR5(OP)                             \
    template <typename U>                         \
    wire<T> &operator OP(const wire<U> &w2) const \
    {                                             \
      w->set(w->get() OP (U)w2);                  \
      return *this;                               \
    }                                             \
                                                  \
    template <typename U>                         \
    wire<T> &operator OP(const U &t) const        \
    {                                             \
      w->set(w->get() OP t);                      \
      return *this;                               \
    }

    OPERATOR5(<<=)
    OPERATOR5(>>=)
  };

  template <typename T>
  std::ostream& operator<<(std::ostream& lhs, wire<T> const& rhs)
  {
    lhs << (T)rhs;
    return lhs;
  }
}

#endif
