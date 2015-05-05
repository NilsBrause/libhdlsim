#ifndef WIRE_HPP
#define WIRE_HPP

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <limits>
#include <set>

#include <base.hpp>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace hdl
{
  template<typename T>
  class wire;

  namespace detail
  {
#ifdef MULTIASSIGN
    // Resolve multiple assignments to a wire.
    // Has to be reimplemented by types with high-Z support.
    template <typename T>
    T resolve(const std::map<hdl::detail::part_base*, T> &candidates,
              const hdl::detail::wire_base *w)
    {
      if(candidates.size() > 1)
        {
          std::cerr << "ERROR: resolve() unimplemented for type "
                    << typeid(T).name() << ". Drivers: " << std::endl;
          for(auto d : candidates)
            std::cout << "  " << (!d.first ? "NULL" : d.first->getname()) << std::endl;
        }
      return candidates.begin()->second;
    }
#endif

    template <typename T>
    class wire_int : public wire_base
    {
    private:
      T state;
      T prev_state;
#ifdef MULTIASSIGN
      std::map<part_base*, T> drivers;
#else
      T next_state;
      bool been_set;
#ifdef DEBUG
      std::set<part_base*> drivers;
#endif
#endif
      bool first;
      std::map<part_base*, bool> seen_event;

      virtual void update()
      {
#ifdef MULTIASSIGN
        if(drivers.size() > 0)
          {
            prev_state = state;
            state = resolve(drivers, this);
            for(auto &c : seen_event)
              c.second = false;
          }
#else
        if(been_set)
          {
            prev_state = state;
            state = next_state;
            for(auto &c : seen_event)
              c.second = false;
            been_set = false;
          }
#endif
      }
      
      bool changed()
      {
#ifdef MULTIASSIGN
        if(drivers.size() > 0)
          return resolve(drivers, this) != state;
        else
          return false;
#else            
        return (next_state != state);
#endif
      }

      void set(const T &t)
      {
#ifdef MULTIASSIGN
        part_base *p = get_cur_part();
        lock();
        drivers[p] = t;
        unlock();
#else
#ifdef DEBUG
        part_base *p = get_cur_part();
        lock();
        drivers.insert(p);
        if(drivers.size() > 1)
          {
            std::cerr << "ERROR: Wire \"" << getname() << "\" has multiple drivers: " << std::endl;
            for(auto d : drivers)
              std::cout << "  " << (!d ? "NULL" : d->getname()) << (d == p ? "*" : "" ) << std::endl;
          }
        unlock();
#endif
        next_state = t;
        been_set = true;
#endif
      }
      
      T get()
      {
        return state;
      }

      bool event()
      {
        part_base *p = get_cur_part();
        lock();
        bool seen = seen_event[p];
        seen_event[p] = true;
        unlock();
        return (prev_state != state) && !seen;
      }

      std::string print()
      {
        std::stringstream ss;
        ss << get();
        return ss.str();
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
    wire(T initial = T(), std::string name = "")
      : w(new detail::wire_int<T>(name, initial))
    {
      detail::wires.push_back(w);
    }

    bool event() const
    {
      return w->event();
    }

    // assignment operators

    void operator=(const T &t) const
    {
      w->set(t);
    }

    void operator=(const wire<T> &w2) const
    {
      w->set((T)w2);
    }

    // conversion operators

    operator T() const
    {
      return w->get();
    }

    operator std::shared_ptr<detail::wire_base>() const
    {
      return w;
    }

    operator std::list<std::shared_ptr<detail::wire_base>>() const
    {
      std::list<std::shared_ptr<detail::wire_base> > result;
      result.push_back(w);
      return result;
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
      return w->get() OP w2;                 \
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
      w->set(w->get() OP w2);                     \
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
      return w->get() OP w2;                 \
    }                                        \
                                             \
    template <typename U>                    \
    RET operator OP(const U &t) const        \
    {                                        \
      return w->get() OP t;                  \
    }

    OPERATOR4(T, <<);
    OPERATOR4(T, >>);

    // modifying two-type binary operators

#define OPERATOR5(OP)                             \
    template <typename U>                         \
    wire<T> &operator OP(const wire<U> &w2) const \
    {                                             \
      w->set(w->get() OP w2);                     \
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

  template <typename T, unsigned int width>
  class bus
  {
  private:
    std::array<wire<T>, width> wires;

  public:
    bus()
    {
    }

    bus(std::initializer_list<T> initial)
    {
      assert(initial.size() == width);
      unsigned int c = width-1;
      for(auto &i : initial)
        wires[c--] = i;
    }

    bus(std::array<T, width> initial)
    {
      unsigned int c = 0;
      for(auto &i : initial)
        wires[c++] = i;
    }

    const wire<T> &operator[](unsigned int n) const
    {
      return wires.at(n);
    }

    const wire<T> &at(unsigned int n) const
    {
      return wires.at(n);
    }

    // assignment operators

    void operator=(const std::array<T, width> &b)
    {
      for(unsigned int c = 0; c < width; c++)
        wires[c] = b[c];
    }

    void operator=(const bus<T, width> &b)
    {
      for(unsigned int c = 0; c < width; c++)
        wires[c] = b[c];
    }

    // conversion operators

    operator std::array<T, width>() const
    {
      std::array<T, width> result;
      for(unsigned int c = 0; c < width; c++)
        result[c] = wires[c];
      return result;
    }

    operator std::list<std::shared_ptr<detail::wire_base>>() const
    {
      std::list<std::shared_ptr<detail::wire_base> > result;
      for(auto &w: wires)
        result.push_back(w);
      return result;
    }
  };

  template <typename T, unsigned int width>
  std::ostream& operator<<(std::ostream& lhs, bus<T, width> const& rhs)
  {
    for(unsigned int c = 0; c < width; c++)
      lhs << rhs[width-c-1];
    return lhs;
  }
}

#endif
