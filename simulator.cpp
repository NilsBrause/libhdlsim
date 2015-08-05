#include <simulator.hpp>

using namespace hdl;

simulator::simulator(part testbench)
  : tb(testbench), cur_time(0)
{}

void simulator::run(uint64_t duration)
{
  std::vector<std::shared_ptr<hdl::detail::wire_base> > wires2up;
  std::vector<std::shared_ptr<hdl::detail::part_base> > procs2up;
  std::shared_ptr<hdl::detail::part_base> testbench = tb.p;
  bool first = true;

  for(; duration > 0; duration--, cur_time++)
    {
#ifdef DEBUG
      std::cerr << "Time: " << cur_time << std::endl;
#endif

      // Run testbench
      testbench->update(cur_time);

      // initialze with wires that have been changed in the testbench
      wires2up.clear();
      if(first)
        {
          for(auto &w : hdl::detail::wires)
            wires2up.push_back(w);
          first = false;
        }
      else
        for(auto &w : testbench->children)
          if(w->changed())
            wires2up.push_back(w);

      // repeat as long as there are wires to be updated.
      while(wires2up.size() > 0)
        {
#ifdef DEBUG
          std::cerr << "Wires to update: " << std::endl;
          for(auto &w : wires2up)
            std::cerr << "  " << w->getname() << std::endl;
#endif

          // update wires
          for(unsigned int c = 0; c < wires2up.size(); c++)
            {
#ifdef DEBUG
              std::cerr << "Updating wire " << wires2up[c]->getname() << std::endl;
#endif
              wires2up[c]->update(cur_time);
            }


          // collect all connected parts
          {
            procs2up.clear();
            for(auto &w : wires2up)
              for(auto &p : w->children)
                procs2up.push_back(p);
            // sort & unique afterwards is actually faster
            std::sort(procs2up.begin(), procs2up.end());
            auto last = std::unique(procs2up.begin(), procs2up.end());
            procs2up.erase(last, procs2up.end());
          }

#ifdef DEBUG
          std::cerr << "Parts to update: " << std::endl;
          for(auto &p : procs2up)
            std::cerr << "  " << p->getname() << std::endl;
#endif

          // update parts
          for(unsigned int c = 0; c < procs2up.size(); c++)
            {
#ifdef DEBUG
              std::cerr << "Updating part " << procs2up[c]->getname() << std::endl;
#endif
              procs2up[c]->update(cur_time);
            }

          // collect all connected wires
          {
            wires2up.clear();
            for(auto &p : procs2up)
              for(auto &w : p->children)
                if(w->changed())
                  wires2up.push_back(w);
            // sort & unique afterwards is actually faster
            std::sort(wires2up.begin(), wires2up.end());
            auto last = std::unique(wires2up.begin(), wires2up.end());
            wires2up.erase(last, wires2up.end());
          }
        }
    }
}

