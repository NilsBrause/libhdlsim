#include <algorithm>
#include <iostream>
#include <base.hpp>

using namespace hdl;

uint64_t base::waitfor(uint64_t duration)
{
  static std::vector<base*> wires2up;
  static std::vector<base*> procs2up;

  time += duration;
  if(duration > 0)
    {
      wires2up.reserve(wires.size());
      procs2up.reserve(processes.size());

      // initialze with wires that have been changed in the testbench
      wires2up.clear();
      for(auto &w : wires)
        if(w->changed())
          wires2up.push_back(w);

      // repeat as long as there are wires to be updated.
#pragma omp parallel num_threads(2)
      while(wires2up.size() > 0)
        {
          // update wires and collect all connected processes

#pragma omp single
          {
            procs2up.clear();
            for(auto &w : wires2up)
              for(auto &p : w->connections)
                if(std::find(procs2up.begin(), procs2up.end(), p)
                   == procs2up.end())
                  procs2up.push_back(p);
          }

#pragma omp for schedule(dynamic)
          for(unsigned int c = 0; c < wires2up.size(); c++)
            wires2up[c]->update();

          // update processes and collect all connected wires

#pragma omp single
          wires2up.clear();

#pragma omp for schedule(dynamic)
          for(unsigned int c = 0; c < procs2up.size(); c++)
            procs2up[c]->update();

#pragma omp single
          {
            for(auto &p : procs2up)
              for(auto &w : p->connections)
                if(w->changed())
                  {
                    if(std::find(wires2up.begin(), wires2up.end(), p)
                       == wires2up.end())
                      wires2up.push_back(w);
                    else
                      std::cerr << "WARNING: wire " << w->myname
                                << " has been updated by more then one process"
                                << " at a time" << std::endl;
                  }
          }
              
        }
    }
  return time;
}

base::base(std::string name)
  : myname(name)
{
}

void base::connect(base* to)
{
  connections.push_back(to);
}

bool base::changed()
{
  return true;
}

uint64_t base::time = 0;
std::list<base*> base::wires;
std::list<base*> base::processes;

