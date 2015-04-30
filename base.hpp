#ifndef BASE_HPP
#define BASE_HPP

#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <sstream>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace hdl
{
  uint64_t waitfor(uint64_t duration);

  namespace detail
  {
    class part_base;

    class root
    {
    private:
      std::string myname;
#ifdef _OPENMP
      static omp_nest_lock_t omp_lock;
#endif
      static std::vector<part_base*> cur_part;

    protected:
      void set_cur_part(part_base *the_part);
      part_base *get_cur_part();
      void lock();
      void unlock();      

    public:
      root(std::string name);
      std::string getname();
    };
      
    template <typename T>
    class base : public root
    {
    protected:
      std::list<std::shared_ptr<T> > parents;
      std::list<std::shared_ptr<T> > children;

      virtual void update() = 0;

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
      
      friend uint64_t hdl::waitfor(uint64_t duration);
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
      friend uint64_t hdl::waitfor(uint64_t duration);

    protected:
      std::string id;
      std::string getid();
      virtual int digits() = 0;
      virtual bool event() = 0;
      virtual std::string print() = 0;
      wire_base(std::string name);

      friend class hdl::detail::part_int;
    };

    extern uint64_t cur_time;
    extern std::list<std::shared_ptr<wire_base> > wires;
    extern std::list<std::shared_ptr<part_base> > parts;
  }
}

#endif
