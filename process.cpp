#include <iostream>
#include <process.hpp>

using namespace hdl;

detail::process_base::process_base(std::list<std::shared_ptr<base> > outputs,
                                   std::function<void()> logic,
                                   std::string name)
  : base(name), logic(logic)
{
  for(auto& outp : outputs)
    connections.push_back(outp);
}

void detail::process_base::update()
{
  logic();
}

process::process(std::list<std::shared_ptr<base> > sensitivity_list,
                 std::list<std::shared_ptr<base> > outputs,
                 std::function<void()> logic, std::string name)
  : p(new detail::process_base(outputs, logic, name))
{
  for(auto &w : sensitivity_list)
    w->connections.push_back(p);
  base::processes.push_back(p);
}
