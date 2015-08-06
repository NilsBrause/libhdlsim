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

hdl::detail::named_obj::named_obj(std::string name)
  : myname(name == "" ? new_tmp() : name)
{
}

std::string hdl::detail::named_obj::getname() const
{
  return myname;
}

void hdl::detail::named_obj::setname(std::string name)
{
  myname = name;
}

//-----------------------------------------------------------------------------

void hdl::detail::base::set_cur_part(hdl::detail::base *the_part)
{
  std::thread::id id = std::this_thread::get_id();
  mutex.lock();
  cur_part[id] = the_part;
  mutex.unlock();
}

hdl::detail::base *hdl::detail::base::get_cur_part()
{
  std::thread::id id = std::this_thread::get_id();
  base *p;
  mutex.lock();
  if(cur_part.size() == 0) // set from top level testbench
    p = NULL;
  else
    p = cur_part[id];
  mutex.unlock();
  return p;
}

std::list<std::shared_ptr<hdl::detail::base> > hdl::detail::wires;
std::list<std::shared_ptr<hdl::detail::base> > hdl::detail::parts;
std::unordered_map<std::thread::id, hdl::detail::base*> hdl::detail::base::cur_part;
std::mutex hdl::detail::base::mutex;
