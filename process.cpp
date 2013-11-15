#include <iostream>
#include <process.hpp>

using namespace hdl;

process::process(std::list<base*> sensitivitylist,
                 std::list<base*> outputs,
                 std::function<void()> logic,
                 std::string name)
  : base(name), logic(logic)
{
  processes.push_back(this);
  for(auto& item : sensitivitylist)
    item->connect(this);
  for(auto& outp : outputs)
    connect(outp);
}

void process::update()
{
#ifdef DEBUG
  std::cout << "process " << myname << " has been run" << std::endl;
#endif
  logic();
}
