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

      template <typename U>
      void set(const U &u)
      {
        T t;
        t = u;
#ifdef MULTIASSIGN
        {
          std::lock_guard<std::mutex> lock(mutex);
          drivers[get_cur_part()] = t;
        }
        if(!changed())
          if(t != state)
            set_changed(true);
          /*
          if(resolve(drivers, this) != state)
            been_changed = true;
          */
#else
#ifdef DEBUG
        {
          std::lock_guard<std::mutex> lock(mutex);
          drivers.insert(get_cur_part());
        }
        if(drivers.size() > 1)
          {
            std::cerr << "ERROR: Wire \"" << getname() << "\" has multiple drivers: " << std::endl;
            for(auto d : drivers)
              std::cout << "  " << (!d ? "NULL" : d->getname()) << std::endl;
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

    template <typename U>
    void operator=(const U &t) const
    {
      w->set(t);
    }

    template <typename U>
    void operator=(const wire<U> &w2) const
    {
      w->set(w2.get());
    }

    // conversion operators

    operator T() const
    {
      return w->get();
    }

    T get() const
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

#define OPERATOR1(OP)                           \
    decltype(OP T()) operator OP() const        \
    {                                           \
      return OP w->get();                       \
    }                                           \

    OPERATOR1(!)
    OPERATOR1(~)
    OPERATOR1(+)
    OPERATOR1(-)

    // binary operators

#define OPERATOR2(OP)                                           \
    template <typename U>                                       \
    decltype(T() OP U()) operator OP(const wire<U> &w2) const   \
    {                                                           \
      return w->get() OP w2.get();                              \
    }                                                           \
                                                                \
    template <typename U>                                       \
    decltype(T() OP U()) operator OP(const U &t) const          \
    {                                                           \
      return w->get() OP t;                                     \
    }

    OPERATOR2(==)
    OPERATOR2(!=)
    OPERATOR2(>)
    OPERATOR2(<)
    OPERATOR2(>=)
    OPERATOR2(<=)
    OPERATOR2(&)
    OPERATOR2(|)
    OPERATOR2(^)
    OPERATOR2(+)
    OPERATOR2(-)
    OPERATOR2(*)
    OPERATOR2(/)
    OPERATOR2(%)
    OPERATOR2(<<)
    OPERATOR2(>>)

    // modifying operators

#define OPERATOR3(OP, OP2)                              \
    template <typename U>                               \
    const wire<T> &operator OP(const wire<U> &w2) const \
    {                                                   \
      w->set(w->get() OP2 w2.get());                    \
      return *this;                                     \
    }                                                   \
                                                        \
    template <typename U>                               \
    const wire<T> &operator OP(const U &t) const        \
    {                                                   \
      w->set(w->get() OP2 static_cast<T>(t));           \
      return *this;                                     \
    }

    OPERATOR3(&=, &)
    OPERATOR3(|=, |)
    OPERATOR3(^=, ^)
    OPERATOR3(+=, +)
    OPERATOR3(-=, -)
    OPERATOR3(*=, *)
    OPERATOR3(/=, /)
    OPERATOR3(%=, %)
    OPERATOR3(<<=, <<)
    OPERATOR3(>>=, >>)
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
  };

  template <typename T, unsigned int width>
  std::ostream& operator<<(std::ostream& lhs, bus<T, width> const& rhs)
  {
    for(unsigned int c = 0; c < width; c++)
      lhs << rhs[width-c-1].get();
    return lhs;
  }

  template <typename T>
  std::ostream& operator<<(std::ostream& lhs, wire<T> const& rhs)
  {
    lhs << rhs.get();
    return lhs;
  }
}

#endif
