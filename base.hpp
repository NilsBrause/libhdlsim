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
      bool been_changed;

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
