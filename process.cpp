#include <iostream>
#include <process.hpp>

using namespace hdl;

detail::process_int::process_int(std::string name,
                                 std::list<std::shared_ptr<detail::wire_base> > sensitivity_list,
                                 std::list<std::shared_ptr<detail::wire_base> > outputs,
                                 std::function<void()> logic)
  : process_base(name), logic(logic)
{
  for(auto &w : outputs)
    children.push_back(w);
  for(auto &w : sensitivity_list)
    parents.push_back(w);
}

void detail::process_int::update()
{
  for(auto &w : children)
    w->lock();
  logic();
  for(auto &w : children)
    w->unlock();
}

process::process(std::string name,
                 std::list<std::shared_ptr<detail::wire_base> > sensitivity_list,
                 std::list<std::shared_ptr<detail::wire_base> > outputs,
                 std::function<void()> logic)
  : p(new detail::process_int(name, sensitivity_list, outputs, logic))
{
  for(auto &w : sensitivity_list)
    w->children.push_back(p);
  for(auto &w : outputs)
    w->parents.push_back(p);
  detail::processes.push_back(p);
}
