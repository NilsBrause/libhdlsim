#ifndef STD_LOGIC_HPP
#define STD_LOGIC_HPP

class std_logic
{
private:
  enum state_t { high, low, highz, undef };
  state_t state;

  std_logic lut2(const std_logic &rhs, state_t hh, state_t hl, state_t lh, state_t ll) const
  {
    std_logic lhs;
    switch(state)
      {
      case high:
        switch(rhs.state)
          {
          case high:
            lhs.state = hh;
            break;
          case low:
            lhs.state = hl;
            break;
          default:
            lhs.state = undef;
            break;
          }
        break;
      case low:
        switch(rhs.state)
          {
          case high:
            lhs.state = lh;
            break;
          case low:
            lhs.state = ll;
            break;
          default:
            lhs.state = undef;
            break;
          }
        break;
      default:
        lhs.state = undef;
        break;
      }
    return lhs;
  }

public:
  std_logic()
    : state(undef)
  {
  }

  std_logic(const bool rhs)
  {
    operator=(rhs);
  }

  std_logic& operator=(const std_logic& rhs)
  {
    state = rhs.state;
    return *this;
  }

  std_logic& operator=(const bool rhs)
  {
    state = (rhs > 0 ? high : low);
    return *this;
  }

  bool operator==(const std_logic& rhs) const
  {
    return state == rhs.state;
  }

  bool operator!=(const std_logic& rhs) const
  {
    return state != rhs.state;
  }

  operator bool() const
  {
    switch(state)
      {
      case high:
        return true;
        break;
      default:
        return false;
        break;
      }
  }

  operator char() const
  {
  switch(state)
    {
    case high:
      return '1';
      break;
    case low:
      return '0';
      break;
    case highz:
      return 'Z';
      break;
    default:
      return 'U';
      break;
    }
  }

  std_logic operator !() const
  {
    std_logic lhs;
    switch(state)
      {
      case high:
        lhs.state = low;
        break;
      case low:
        lhs.state = high;
        break;
      default:
        lhs.state = undef;
        break;
      }
    return lhs;
  }

  std_logic operator ~() const
  {
    return operator not();
  }

  std_logic operator &(const std_logic &rhs) const
  {
    return lut2(rhs, high, low, low, low);
  }

  std_logic operator |(const std_logic &rhs) const
  {
    return lut2(rhs, high, high, high, low);
  }

  std_logic operator ^(const std_logic &rhs) const
  {
    return lut2(rhs, low, high, high, low);
  }
};

std::ostream& operator<<(std::ostream& os, const std_logic& rhs)
{
  os << (char)rhs;
  return os;
}

#ifdef MULTIASSIGN
std_logic resolve(std::map<hdl::detail::part_base*, std_logic> candidates,
                  hdl::detail::wire_base *w)
{
  std::vector<std_logic> nonhz;
  for(auto &i : candidates)
    if((char)i.second != 'Z')
      nonhz.push_back(i.second);
  switch(nonhz.size())
    {
    case 0:
      return std_logic('Z');
      break;
    case 1:
      return nonhz.front();
      break;
    default: // short circuit
      std::cerr << "WARNING: wire " << w->getname()
                << " has been updated by the following parts: ";
      for(auto &i : candidates)
        if(i.first)
          std::cerr << i.first->getname() << " ";
        else
          std::cerr << "testbench ";
      std::cerr << std::endl;
      return std_logic('U');
      break;
    } 
}
#endif

#endif // STD_LOGIC_HPP
