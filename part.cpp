#include <part.hpp>

using namespace hdl;

detail::part_int::part_int(std::list<std::list<std::shared_ptr<detail::base> > > inputs,
                           std::list<std::list<std::shared_ptr<detail::base> > > outputs,
                           std::function<void(uint64_t)> logic)
  : logic(logic)
{
  for(auto &l : outputs)
    for(auto &w : l)
      children.insert(w);
}

void detail::part_int::update(uint64_t time)
{
  set_cur_part(this);
  logic(time);
  set_cur_part(NULL);
  set_changed(false);
}

part::part(std::list<std::list<std::shared_ptr<detail::base> > > inputs,
           std::list<std::list<std::shared_ptr<detail::base> > > outputs,
           std::function<void(uint64_t)> logic,
           std::string name)
  : p(new detail::part_int(inputs, outputs, logic))
{
  p->setname(name);
  for(auto &l : inputs)
    for(auto &w : l)
      w->children.insert(p);
  detail::parts.push_back(p);
}
