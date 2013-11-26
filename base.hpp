#ifndef BASE_HPP
#define BASE_HPP

#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace hdl
{
  uint64_t waitfor(uint64_t duration = 0);

  class process;

  namespace detail
  {
    template <typename T>
    class base
    {
    private:
      std::string myname;

    protected:
      std::list<std::shared_ptr<T> > parents;
      std::list<std::shared_ptr<T> > children;

      virtual void update() = 0;

      base(std::string name)
        : myname(name)
      {
      }

      void add_child(std::shared_ptr<T> child)
      {
        children.push_back(child);
      }

      void add_parent(std::shared_ptr<T> parent)
      {
        parents.push_back(parent);
      }
      
      friend uint64_t hdl::waitfor(uint64_t duration);
      friend class hdl::process;

    public:
      std::string getname()
      {
        return myname;
      }
    };
    
    class wire_base;

    class process_base : public base<wire_base>
    {
    protected:
      process_base(std::string name);
    };

    class wire_base : public base<process_base>
    {
    private:
#ifdef _OPENMP
      omp_nest_lock_t omp_lock;
#endif
      virtual bool changed() = 0;
      friend uint64_t hdl::waitfor(uint64_t duration);

    protected:
      std::vector<process_base*> cur_parent;
      void lock();
      void unlock();
      
void set_cur_parent(process_base *parent);
      wire_base(std::string name);

      friend class process_int;

    public:
    };    

    extern uint64_t cur_time;
    extern std::list<std::shared_ptr<wire_base> > wires;
    extern std::list<std::shared_ptr<process_base> > processes;
  }
}

#endif
