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

std::vector<std::shared_ptr<hdl::detail::base> > hdl::detail::wires;
std::vector<std::shared_ptr<hdl::detail::base> > hdl::detail::parts;
thread_local hdl::detail::base* hdl::detail::base::cur_part;
