#ifndef WIRE_HPP
#define WIRE_HPP

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
    T resolve(std::map<hdl::detail::part_base*, T> candidates,
              hdl::detail::wire_base *w)
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
      std::map<part_base*, T> next_state;
      std::map<part_base*, T> drivers;
#else
      T next_state;
      std::set<part_base*> drivers;
#endif
      bool first;
      std::set<part_base*> seen_event;

      virtual void update()
      {
#ifdef MULTIASSIGN
        if(next_state.size() > 0)
          {
            prev_state = state;
            for(auto &i : next_state)
              drivers[i.first] = i.second;
            state = resolve(drivers, this);
            next_state.clear();
          }
#else
        prev_state = state;
        state = next_state;
#endif
        seen_event.clear();
      }
      
      bool changed()
      {
        if(!first)
          {
#ifdef MULTIASSIGN
            if(next_state.size() > 0)
              {
                std::map<part_base*, T> tmp = drivers;
                for(auto &i : next_state)
                  tmp[i.first] = i.second;
                return resolve(tmp, this) != state;
              }
            else
              return false;
#else            
            return (next_state != state);
#endif
          }
        else
          {
            first = false;
            return true;
          }
      }

      void set(const T &t)
      {
#ifdef MULTIASSIGN
        part_base *p = get_cur_part();
        lock();
        next_state[p] = t;
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
        bool not_seen = seen_event.insert(p).second;
        unlock();
        return (prev_state != state) && not_seen;
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
    wire(T initial)
      : w(new detail::wire_int<T>("", initial))
    {
      detail::wires.push_back(w);
    }

    wire(std::string name)
      : w(new detail::wire_int<T>(name, T()))
    {
      detail::wires.push_back(w);
    }

    wire(std::string name, T initial)
      : w(new detail::wire_int<T>(name, initial))
    {
      detail::wires.push_back(w);
    }

    void replace(const wire<T> &w2)
    {
      w = w2.w;
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

    // conversion operators
    
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
