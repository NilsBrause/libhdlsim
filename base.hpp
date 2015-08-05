#ifndef BASE_HPP
#define BASE_HPP

#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <thread>
#include <mutex>

namespace hdl
{
  template <typename T>
  constexpr T power(T base, unsigned int exp)
  {
    return exp == 0 ? 1 : base * power(base, exp-1);
  }

  class simulator;

  namespace detail
  {
    // misc helpers
    template <bool B, class T = void>
    struct enable_if
    {
      typedef T type;
    };

    template <class T>
    struct enable_if<false, T>
    {
    };

    class part_base;

    class root
    {
    private:
      std::string myname;

      static std::mutex mutex;
      static std::unordered_map<std::thread::id, part_base*> cur_part;

    protected:
      void set_cur_part(part_base *the_part);
      part_base *get_cur_part();
      void lock();
      void unlock();      

    public:
      root(std::string name);
      std::string getname() const;
    };

    template <typename T>
    class base : public root
    {
    protected:
      std::list<std::shared_ptr<T> > parents;
      std::list<std::shared_ptr<T> > children;

      virtual void update(uint64_t time) = 0;
      friend class hdl::simulator;

      base(std::string name)
        : root(name)
      {
      }

    public:
      void add_child(std::shared_ptr<T> child)
      {
        if(std::find(children.begin(), children.end(), child) == children.end())
          children.push_back(child);
      }

      void add_parent(std::shared_ptr<T> parent)
      {
        if(std::find(parents.begin(), parents.end(), parent) == parents.end())
          parents.push_back(parent);
      }
    };
    
    class wire_base;

    class part_base : public base<wire_base>
    {
    protected:
      part_base(std::string name);
    };

    class part_int;

    class wire_base : public base<part_base>
    {
    private:
      virtual bool changed() = 0;
      friend class hdl::simulator;

    protected:
      std::string id;
      std::string getid();
      virtual int digits() = 0;
      virtual bool event() = 0;
      virtual std::string print() = 0;
      wire_base(std::string name);

      friend class hdl::detail::part_int;
    };

    extern std::list<std::shared_ptr<wire_base> > wires;
    extern std::list<std::shared_ptr<part_base> > parts;
  }
}

#endif
