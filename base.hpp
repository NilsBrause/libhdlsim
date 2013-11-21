#ifndef BASE_HPP
#define BASE_HPP

#include <list>
#include <memory>
#include <string>
#include <cstdint>

namespace hdl
{
  class base
  {
  private:
    static uint64_t time;

  protected:
    std::string myname;
    virtual bool changed();
    virtual void update() = 0;

    static std::list<std::shared_ptr<base> > wires;
    static std::list<std::shared_ptr<base> > processes;

    base(std::string name = "unknowen");

  public:
    std::list<std::shared_ptr<base> > connections;

    static uint64_t waitfor(uint64_t duration = 0);
  };
}

#endif
