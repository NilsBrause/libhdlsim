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

#ifndef BASE_HPP
#define BASE_HPP

#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace hdl
{
  template <typename T>
  constexpr T power(T base, unsigned int exp)
  {
    return exp == 0 ? 1 : base * power(base, exp-1);
  }

  class simulator;
  class part;

  void cleanup();

  namespace detail
  {
    // misc helpers
    template <bool B, class T = void>
    struct enable_if
    {
      typedef T type;
    };

    template <class T>
    struct enable_if<false, T>
    {
    };

    class named_obj
    {
    private:
      std::string myname;

    public:
      named_obj(std::string name = "");
      std::string getname() const;
      virtual void setname(std::string name);
    };

    class base : public named_obj
    {
    private:
      static thread_local base* cur_part;
      bool been_changed = false;

    protected:
      std::unordered_set<std::shared_ptr<base> > children;

      inline void set_cur_part(base *the_part) { cur_part = the_part; }
      inline base *get_cur_part() { return cur_part; }

      inline void set_changed(bool b) { been_changed = b; }
      inline bool changed() { return been_changed; }

      virtual void update(uint64_t time) = 0;

      friend class hdl::simulator;
      friend class hdl::part;
    };
    
    extern std::vector<std::shared_ptr<base> > wires;
    extern std::vector<std::shared_ptr<base> > parts;
  }
}

#endif
