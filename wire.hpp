#ifndef WIRE_HPP
#define WIRE_HPP

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <limits>
#include <set>

#include <base.hpp>

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
    T resolve(const std::map<detail::base*, T> &candidates,
              const detail::base *w)
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
  }

  template<typename T>
  class wire
  {
  private:
    class wire_int : public detail::base
    {
    private:
      T state;
      T prev_state;
#ifdef MULTIASSIGN
      std::map<base*, T> drivers;
#else
      T next_state;
      bool been_set;
#ifdef DEBUG
      std::set<base*> drivers;
#endif
#endif
      bool first;
      std::map<base*, bool> seen_event;
      std::mutex mutex;

      virtual void update(uint64_t)
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
        set_changed(false);
      }

      void set(const T &t)
      {
        std::lock_guard<std::mutex> lock(mutex);
#ifdef MULTIASSIGN
        drivers[get_cur_part()] = t;
        if(!changed())
          if(t != state)
            set_changed(true);
          /*
          if(resolve(drivers, this) != state)
            been_changed = true;
          */
#else
#ifdef DEBUG
        drivers.insert(get_cur_part());
        if(drivers.size() > 1)
          {
            std::cerr << "ERROR: Wire \"" << getname() << "\" has multiple drivers: " << std::endl;
            for(auto d : drivers)
              std::cout << "  " << (!d ? "NULL" : d->getname()) << (d == p ? "*" : "" ) << std::endl;
          }
#endif
        next_state = t;
        been_set = true;
        if(!changed())
          if(next_state != state)
            set_changed(true);
#endif
      }
      
      T get()
      {
        return state;
      }

      bool event()
      {
        std::lock_guard<std::mutex> lock(mutex);
        bool seen = seen_event[get_cur_part()];
        seen_event[get_cur_part()] = true;
        return (prev_state != state) && !seen;
      }

      std::string print()
      {
        std::stringstream ss;
        ss << get();
        return ss.str();
      }
      
      wire_int()
        : state(T()), prev_state(T()),
#ifndef MULTIASSIGN
          been_set(false),
#endif
          first(true)
      {
      }
      
      friend class wire<T>;
    };

    std::shared_ptr<wire_int> w;

  public:
    wire()
      : w(new wire_int())
    {
      detail::wires.push_back(w);
    }

    wire(T initial)
      : w(new wire_int())
    {
      w->set(initial);
      detail::wires.push_back(w);
    }

    bool event() const
    {
      return w->event();
    }

    std::string getname() const
    {
      return w->getname();
    }

    void setname(std::string name)
    {
      w->setname(name);
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

    operator std::shared_ptr<detail::base>() const
    {
      return w;
    }

    operator std::list<std::shared_ptr<detail::base>>() const
    {
      std::list<std::shared_ptr<detail::base> > result;
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

    OPERATOR3(&=)
    OPERATOR3(|=)
    OPERATOR3(^=)
    OPERATOR3(+=)
    OPERATOR3(-=)
    OPERATOR3(*=)
    OPERATOR3(/=)
    OPERATOR3(%=)

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

    OPERATOR4(T, <<)
    OPERATOR4(T, >>)

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
    class bus_int : public detail::named_obj
    {
    private:
      static_assert(width > 0, "width > 0");
      std::array<wire<T>, width> wires;

    public:
      bus_int()
        : named_obj("")
      {
      }

      const wire<T> &at(unsigned int n) const
      {
        return wires.at(n);
      }

      void setname(std::string name)
      {
        for(unsigned int c = 0; c < wires.size(); c++)
          wires[c].setname(name + "[" + std::to_string(c) + "]");
        named_obj::setname(name);
      }
    };

    std::shared_ptr<bus_int> b;

  public:
    bus()
      : b(new bus_int())
    {
    }

    bus(std::initializer_list<T> initial)
      : b(new bus_int())
    {
      assert(initial.size() == width);
      unsigned int c = width-1;
      for(auto &i : initial)
        b->at(c--) = i;
    }

    bus(std::array<T, width> initial)
      : b(new bus_int())
    {
      operator=(initial);
    }

    template <typename U, typename detail::enable_if<std::is_integral<U>::value, int>::type dummy = 0>
    bus(U initial)
      : b(new bus_int())
    {
      operator=(initial);
    }

    void setname(std::string name)
    {
      b->setname(name);
    }

    std::string getname() const
    {
      return b->getname();
    }

    const wire<T> &operator[](unsigned int n) const
    {
      return b->at(n);
    }

    const wire<T> &at(unsigned int n) const
    {
      return b->at(n);
    }

    // assignment operators

    void operator=(const std::array<T, width> &b2) const
    {
      for(unsigned int c = 0; c < width; c++)
        b->at(c) = b2[c];
    }

    void operator=(const bus<T, width> &b2) const
    {
      for(unsigned int c = 0; c < width; c++)
        b->at(c) = b2[c];
    }

    template <typename U>
    typename detail::enable_if<std::is_integral<U>::value>::type
    operator=(const U value) const
    {
      if(width >= sizeof(U)*8)
        {
          for(unsigned int c = 0; c < sizeof(U)*8; c++)
            b->at(c) = power(2, c) & value ? 1 : 0;
          if(width > sizeof(U)*8)
            {
              if(std::is_signed<U>::value)
                for(unsigned int c = sizeof(U)*8; c < width; c++)
                  b->at(c) = power(2, sizeof(U)*8-1) & value ? 1 : 0;
              else
                for(unsigned int c = sizeof(U)*8; c < width; c++)
                  b->at(c) = 0;
            }
        }
      else
        {
#ifdef DEBUG
          std::cerr << "WARNING: Bus width is to small for integer constant: " << std::endl;
#endif
          for(unsigned int c = 0; c < width; c++)
            b->at(c) = power(2, c) & value ? 1 : 0;
        }
    }

    // conversion operators

    operator std::array<T, width>() const
    {
      std::array<T, width> result;
      for(unsigned int c = 0; c < width; c++)
        result[c] = b->at(c);
      return result;
    }

    operator std::list<std::shared_ptr<detail::base>>() const
    {
      std::list<std::shared_ptr<detail::base> > result;
      for(unsigned int c = 0; c < width; c++)
        result.push_back(b->at(c));
      return result;
    }

    template <typename U, typename detail::enable_if<std::is_integral<U>::value, int>::type dummy = 0>
    operator U() const
    {
      static_assert(sizeof(U)*8 >= width, "sizeof(U)*8 >= width");
      U result = 0;
      for(unsigned int c = 0; c < sizeof(U)*8; c++)
        {
          unsigned int d = sizeof(U)*8-c-1;
          result <<= 1;
          if(d >= width)
            {
              if(std::is_signed<U>::value)
                result |= b->at(width-1) == static_cast<T>(1) ? 1 : 0;
            }
          else
            result |= b->at(d) == static_cast<T>(1) ? 1 : 0;
        }
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
