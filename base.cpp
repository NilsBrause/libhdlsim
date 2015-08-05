#include <algorithm>
#include <iostream>
#include <base.hpp>

uint64_t hdl::waitfor(uint64_t duration)
{
  static std::vector<std::shared_ptr<hdl::detail::wire_base> > wires2up;
  static std::vector<std::shared_ptr<hdl::detail::part_base> > procs2up;
  static bool first = true;

  hdl::detail::cur_time += duration;
  if(duration > 0)
    {
#ifdef DEBUG
      std::cerr << "Time: " << hdl::detail::cur_time << std::endl;
#endif

      // initialze with wires that have been changed in the testbench
#ifdef DEBUG
      std::cerr << "Scanning for wires." << std::endl;
#endif
      wires2up.clear();
      if(first)
        {
          for(auto &w : hdl::detail::wires)
            wires2up.push_back(w);
          first = false;
        }
      else
        for(auto &w : hdl::detail::wires)
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
              wires2up[c]->update();
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
              procs2up[c]->update();
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
}

std::string hdl::detail::root::getname() const
{
  return myname;
}

void hdl::detail::root::set_cur_part(hdl::detail::part_base *the_part)
{
  std::thread::id id = std::this_thread::get_id();
  lock();
  cur_part[id] = the_part;
  unlock();
}

hdl::detail::part_base *hdl::detail::root::get_cur_part()
{
  std::thread::id id = std::this_thread::get_id();
  part_base *p;
  lock();
  if(cur_part.size() == 0) // set from top level testbench
    p = NULL;
  else
    p = cur_part[id];
  unlock();
  return p;
}

void hdl::detail::root::lock()
{
  mutex.lock();
}

void hdl::detail::root::unlock()
{
  mutex.unlock();
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
std::unordered_map<std::thread::id, hdl::detail::part_base*> hdl::detail::root::cur_part;
std::mutex hdl::detail::root::mutex;
