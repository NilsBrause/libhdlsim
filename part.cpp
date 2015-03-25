#include <part.hpp>

using namespace hdl;

detail::part_int::part_int(std::list<std::shared_ptr<detail::wire_base> > inputs,
                           std::list<std::shared_ptr<detail::wire_base> > outputs,
                           std::function<void()> logic,
                           std::string name)
  : part_base(name), logic(logic)
{
  for(auto &w : outputs)
    add_child(w);
  for(auto &w : inputs)
    add_parent(w);
}

void detail::part_int::update()
{
  set_cur_part(this);
  logic();
  set_cur_part(NULL);
}

part::part(std::list<std::shared_ptr<detail::wire_base> > inputs,
           std::list<std::shared_ptr<detail::wire_base> > outputs,
           std::function<void()> logic,
           std::string name)
  : p(new detail::part_int(inputs, outputs, logic, name))
{
  for(auto &w : inputs)
    w->add_child(p);
  for(auto &w : outputs)
    w->add_parent(p);
  detail::parts.push_back(p);
}
