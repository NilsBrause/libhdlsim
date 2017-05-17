/******************************************************************************
 * Copyright (c) 2015-2016, Nils Christopher Brause
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <part.hpp>

using namespace hdl;

detail::part_int::part_int(std::list<std::list<std::shared_ptr<detail::base> > > outputs,
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
  : p(new detail::part_int(outputs, logic))
{
  p->setname(name);
  for(auto &l : inputs)
    for(auto &w : l)
      w->children.insert(p);
  detail::parts.push_back(p);
}
