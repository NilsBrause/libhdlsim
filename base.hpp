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

  namespace detail
  {
    class base
    {
    private:
      std::string myname;

    protected:
      base(std::string name);

    public:
      std::string getname();
    };
    
    class wire_base;

    class process_base : public base
    {
    private:
      std::list<std::shared_ptr<wire_base> > parents;
      virtual void update() = 0;
      friend uint64_t hdl::waitfor(uint64_t duration);

    protected:
      std::list<std::shared_ptr<wire_base> > children;
      process_base(std::string name);

    public:
      void add_child(std::shared_ptr<wire_base> child);
      void add_parent(std::shared_ptr<wire_base> parent);
    };

    class wire_base : public base
    {
    private:
#ifdef _OPENMP
      omp_nest_lock_t omp_lock;
#endif
      std::list<std::shared_ptr<process_base> > children;
      std::list<std::shared_ptr<process_base> > parents;
      virtual bool changed() = 0;
      virtual void update() = 0;
      friend uint64_t hdl::waitfor(uint64_t duration);

    protected:
      std::vector<process_base*> cur_parent;
      wire_base(std::string name);

    public:
      void add_child(std::shared_ptr<process_base> child);
      void add_parent(std::shared_ptr<process_base> parent);
      void lock();
      void unlock();
      void set_cur_parent(process_base *parent);
    };    

    extern uint64_t cur_time;
    extern std::list<std::shared_ptr<wire_base> > wires;
    extern std::list<std::shared_ptr<process_base> > processes;
  }
}

#endif
