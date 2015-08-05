#include <algorithm>
#include <iostream>
#include <base.hpp>

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

std::list<std::shared_ptr<hdl::detail::wire_base> > hdl::detail::wires;
std::list<std::shared_ptr<hdl::detail::part_base> > hdl::detail::parts;
std::unordered_map<std::thread::id, hdl::detail::part_base*> hdl::detail::root::cur_part;
std::mutex hdl::detail::root::mutex;
