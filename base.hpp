#ifndef BASE_HPP
#define BASE_HPP

#include <list>
#include <memory>
#include <string>
#include <cstdint>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace hdl
{
  uint64_t waitfor(uint64_t duration = 0);

  namespace detail
  {
    class wire_base;

    class process_base
    {
    protected:
      process_base(std::string name);
      
      friend uint64_t waitfor(uint64_t duration);

    public:
      std::string myname;
      virtual void update() = 0;
      std::list<std::shared_ptr<wire_base> > children;
      std::list<std::shared_ptr<wire_base> > parents;
    };

    class wire_base
    {
    protected:
#ifdef _OPENMP
      omp_nest_lock_t omp_lock;
#endif
      wire_base(std::string name);
      friend uint64_t waitfor(uint64_t duration);

    public:
      std::string myname;
      virtual bool changed() = 0;
      virtual void update() = 0;
      void lock();
      void unlock();
      std::list<std::shared_ptr<process_base> > children;
      std::list<std::shared_ptr<process_base> > parents;
    };    

    extern uint64_t cur_time;
    extern std::list<std::shared_ptr<wire_base> > wires;
    extern std::list<std::shared_ptr<process_base> > processes;
  }
}

#endif
