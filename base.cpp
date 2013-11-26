#include <algorithm>
#include <iostream>
#include <base.hpp>

uint64_t hdl::waitfor(uint64_t duration)
{
  static std::vector<std::shared_ptr<hdl::detail::wire_base> > wires2up;
  static std::vector<std::shared_ptr<hdl::detail::process_base> > procs2up;

  hdl::detail::cur_time += duration;
  if(duration > 0)
    {
#ifdef DEBUG
      std::cerr << "Time: " << hdl::detail::cur_time << std::endl;
#endif

      wires2up.reserve(hdl::detail::wires.size());
      procs2up.reserve(hdl::detail::processes.size());

      // initialze with wires that have been changed in the testbench
      wires2up.clear();
      for(auto &w : hdl::detail::wires)
        if(w->changed())
          wires2up.push_back(w);

      // repeat as long as there are wires to be updated.
#ifdef _OPENMP
#pragma omp parallel num_threads(2)
#endif
      while(wires2up.size() > 0)
        {
          // update wires and collect all connected processes

#ifdef DEBUG
          std::cerr << "Wires to update: " << std::endl;
          for(auto &w : wires2up)
            std::cerr << "  " << w->myname << std::endl;
#endif

#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
          for(unsigned int c = 0; c < wires2up.size(); c++)
            {
#ifdef DEBUG
              std::cerr << "Updating wire " << wires2up[c]->myname << std::endl;
#endif
              wires2up[c]->update();
            }

#ifdef _OPENMP
#pragma omp single
#endif
          {
            procs2up.clear();
            for(auto &w : wires2up)
              for(auto &p : w->children)
                if(std::find(procs2up.begin(), procs2up.end(), p)
                   == procs2up.end())
                  {
#ifdef DEBUG
                    std::cerr << "Adding process " << p->myname << std::endl;
#endif
                    procs2up.push_back(p);
                  }
          }

          // update processes and collect all connected wires

#ifdef DEBUG
          std::cerr << "Processes to update: " << std::endl;
          for(auto &p : procs2up)
            std::cerr << "  " << p->myname << std::endl;
#endif

#ifdef _OPENMP
#pragma omp single
#endif
          wires2up.clear();

#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
          for(unsigned int c = 0; c < procs2up.size(); c++)
            {
#ifdef DEBUG
              std::cerr << "Updating process " << procs2up[c]->myname << std::endl;
#endif
              procs2up[c]->update();
            }

#ifdef _OPENMP
#pragma omp single
#endif
          {
            for(auto &p : procs2up)
              for(auto &w : p->children)
                if(w->changed())
                  {
                    if(std::find(wires2up.begin(), wires2up.end(), w)
                       == wires2up.end())
                      {
#ifdef DEBUG
                        std::cerr << "Adding wire " << w->myname << std::endl;
#endif
                        wires2up.push_back(w);
                      }
                  }
                else
                  {
#ifdef DEBUG
                    std::cerr << "Wire " << w->myname << " didn't change." << std::endl;
#endif
                  }
          }
#ifdef DEBUG
          std::cerr << std::endl;
#endif
        }
    }
  return hdl::detail::cur_time;
}

hdl::detail::process_base::process_base(std::string name)
  : base(name)
{
}

hdl::detail::wire_base::wire_base(std::string name)
  : base(name)
{
#ifdef _OPENMP
  omp_init_nest_lock(&omp_lock);
#endif
}

void hdl::detail::wire_base::lock()
{
#ifdef _OPENMP
  omp_set_nest_lock(&omp_lock);
#endif
}

void hdl::detail::wire_base::unlock()
{
#ifdef _OPENMP
  omp_unset_nest_lock(&omp_lock);
#endif
}

void hdl::detail::wire_base::set_cur_parent(hdl::detail::process_base *parent)
{
  lock();
#ifdef _OPENMP
  unsigned int n = omp_get_thread_num();
#else
  unsigned int n = 0;
#endif
  if(cur_parent.size() < n+1)
    cur_parent.resize(n+1);
  cur_parent[n] = parent;
  unlock();
}

uint64_t hdl::detail::cur_time = 0;
std::list<std::shared_ptr<hdl::detail::wire_base> > hdl::detail::wires;
std::list<std::shared_ptr<hdl::detail::process_base> > hdl::detail::processes;
