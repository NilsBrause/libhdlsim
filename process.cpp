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
    add_child(w);
  for(auto &w : sensitivity_list)
    add_parent(w);
}

void detail::process_int::update()
{
  for(auto &w : children)
    w->set_cur_parent(this);
  logic();
}

process::process(std::string name,
                 std::list<std::shared_ptr<detail::wire_base> > sensitivity_list,
                 std::list<std::shared_ptr<detail::wire_base> > outputs,
                 std::function<void()> logic)
  : p(new detail::process_int(name, sensitivity_list, outputs, logic))
{
  for(auto &w : sensitivity_list)
    w->add_child(p);
  for(auto &w : outputs)
    w->add_parent(p);
  detail::processes.push_back(p);
}

process::process(std::string name,
                 std::list<std::shared_ptr<detail::wire_base> > sensitivity_list,
                 std::list<std::shared_ptr<detail::wire_base> > inputs,
                 std::list<std::shared_ptr<detail::wire_base> > outputs,
                 std::function<void()> logic)
  : p(new detail::process_int(name, sensitivity_list, outputs, logic))
{
  for(auto &w : sensitivity_list)
    w->add_child(p);
  for(auto &w : inputs)
    w->add_child(p);
  for(auto &w : outputs)
    w->add_parent(p);
  detail::processes.push_back(p);
}
