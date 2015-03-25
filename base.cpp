#include <algorithm>
#include <iostream>
#include <base.hpp>

uint64_t hdl::waitfor(uint64_t duration)
{
  static std::vector<std::shared_ptr<hdl::detail::wire_base> > wires2up;
  static std::vector<std::shared_ptr<hdl::detail::part_base> > procs2up;

  hdl::detail::cur_time += duration;
  if(duration > 0)
    {
#ifdef DEBUG
      std::cerr << "Time: " << hdl::detail::cur_time << std::endl;
#endif

      wires2up.reserve(hdl::detail::wires.size());
      procs2up.reserve(hdl::detail::parts.size());

      // initialze with wires that have been changed in the testbench
#ifdef DEBUG
      std::cerr << "Scanning for wires." << std::endl;
#endif
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
          // update wires and collect all connected parts

#ifdef DEBUG
          std::cerr << "Wires to update: " << std::endl;
          for(auto &w : wires2up)
            std::cerr << "  " << w->getname() << std::endl;
#endif

#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
          for(unsigned int c = 0; c < wires2up.size(); c++)
            {
#ifdef DEBUG
              std::cerr << "Updating wire " << wires2up[c]->getname() << std::endl;
#endif
              wires2up[c]->update();
            }

#ifdef _OPENMP
#pragma omp single
#endif
          {
#ifdef DEBUG
            std::cerr << "Scanning for parts." << std::endl;
#endif
            procs2up.clear();
            for(auto &w : wires2up)
              for(auto &p : w->children)
                if(std::find(procs2up.begin(), procs2up.end(), p)
                   == procs2up.end())
                  {
#ifdef DEBUG
                    std::cerr << "Adding part " << p->getname() << std::endl;
#endif
                    procs2up.push_back(p);
                  }
          }

          // update parts and collect all connected wires

#ifdef DEBUG
          std::cerr << "Parts to update: " << std::endl;
          for(auto &p : procs2up)
            std::cerr << "  " << p->getname() << std::endl;
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
              std::cerr << "Updating part " << procs2up[c]->getname() << std::endl;
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
                        std::cerr << "Adding wire " << w->getname() << std::endl;
#endif
                        wires2up.push_back(w);
                      }
                  }
                else
                  {
#ifdef DEBUG
                    std::cerr << "Wire " << w->getname() << " didn't change." << std::endl;
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

std::string new_tmp()
{
  static int n = 0;
  std::stringstream ss;
  ss << "unnamed" << n++;
  return ss.str();
}

hdl::detail::root::root(std::string name)
  : myname(name == "" ? new_tmp() : name)
{
#ifdef _OPENMP
  omp_init_nest_lock(&omp_lock);
#endif
}

std::string hdl::detail::root::getname()
{
  return myname;
}

void hdl::detail::root::set_cur_part(hdl::detail::part_base *the_part)
{
#ifdef _OPENMP
  unsigned int n = omp_get_thread_num();
#else
  unsigned int n = 0;
#endif
  lock();
  if(cur_part.size() < n+1)
    cur_part.resize(n+1);
  cur_part[n] = the_part;
  unlock();
}

hdl::detail::part_base *hdl::detail::root::get_cur_part()
{
#ifdef _OPENMP
  unsigned int n = omp_get_thread_num();
#else
  unsigned int n = 0;
#endif
  part_base *p;
  lock();
  if(cur_part.size() == 0) // set from top level testbench
    p = NULL;
  else
    p = cur_part.at(n);
  unlock();
  return p;
}

void hdl::detail::root::lock()
{
#ifdef _OPENMP
  omp_set_nest_lock(&omp_lock);
#endif
}

void hdl::detail::root::unlock()
{
#ifdef _OPENMP
  omp_unset_nest_lock(&omp_lock);
#endif
}

hdl::detail::part_base::part_base(std::string name)
  : base(name)
{
}

hdl::detail::wire_base::wire_base(std::string name)
  : base(name)
{
}

uint64_t hdl::detail::cur_time = 0;
std::list<std::shared_ptr<hdl::detail::wire_base> > hdl::detail::wires;
std::list<std::shared_ptr<hdl::detail::part_base> > hdl::detail::parts;
std::vector<hdl::detail::part_base*> hdl::detail::root::cur_part;
#ifdef _OPENMP
omp_nest_lock_t hdl::detail::root::omp_lock;
#endif
