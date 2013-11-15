#ifndef BASE_HPP
#define BASE_HPP

#include <list>
#include <string>

namespace hdl
{
  class process;

  class base
  {
  private:
    std::list<base*> connections;
    static uint64_t time;

  protected:
    std::string myname;
    static std::list<base*> wires;
    static std::list<base*> processes;
    virtual bool changed();
    virtual void update() = 0;
    void connect(base* to);
    friend class process;

  public:
    base(std::string name = "unknowen");
    static uint64_t waitfor(uint64_t duration = 0);
  };
}

#endif
