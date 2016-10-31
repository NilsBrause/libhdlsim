/******************************************************************************
 * Copyright (c) 2015-2016, Nils Christopher Brause
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <simulator.hpp>

using namespace hdl;

simulator::simulator(part testbench)
  : tb(testbench), cur_time(0)
{}

void simulator::run(uint64_t duration)
{
  std::vector<std::shared_ptr<hdl::detail::base> > wires2up;
  std::vector<std::shared_ptr<hdl::detail::base> > procs2up;
  std::shared_ptr<hdl::detail::base> testbench = tb.p;
  static bool first = true;

  for(; duration > 0; duration--, cur_time++)
    {
#ifdef DEBUG
      std::cerr << "Time: " << cur_time << std::endl;
#endif

      // Run testbench
      testbench->update(cur_time);

      // initialze with wires that have been changed in the testbench
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
              for(auto &p : wires2up[c]->children)
                if(!p->changed())
                  {
                    p->set_changed(true);
                    procs2up.push_back(p);
                  }
            }
          wires2up.clear();

          // sort & unique
          std::sort(procs2up.begin(), procs2up.end());
          auto lastproc = std::unique(procs2up.begin(), procs2up.end());
          procs2up.erase(lastproc, procs2up.end());

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
              for(auto &w : procs2up[c]->children)
                if(w->changed())
                  wires2up.push_back(w);
            }
          procs2up.clear();

          // sort & unique
          std::sort(wires2up.begin(), wires2up.end());
          auto lastwire = std::unique(wires2up.begin(), wires2up.end());
          wires2up.erase(lastwire, wires2up.end());
        }
    }
}
