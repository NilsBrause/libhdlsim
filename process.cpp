#include <iostream>
#include <process.hpp>

using namespace hdl;

process::process(std::list<std::shared_ptr<base> > outputs,
                 std::function<void()> logic,
                 std::string name)
  : base(name), logic(logic)
{
  for(auto& outp : outputs)
    connections.push_back(outp);
}

void process::update()
{
  logic();
}

std::shared_ptr<process>
process::create(std::list<std::shared_ptr<base> > sensitivity_list,
                std::list<std::shared_ptr<base> > outputs,
                std::function<void()> logic, std::string name)
{
  std::shared_ptr<process> p(new process(outputs, logic, name));
  for(auto &w : sensitivity_list)
    w->connections.push_back(p);
  base::processes.push_back(p);
  return p;
}
