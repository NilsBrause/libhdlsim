#ifndef STD_LOGIC_HPP
#define STD_LOGIC_HPP

class std_logic
{
private:
  enum state_t { high, low, highz, undef };
  state_t state;

public:
  std_logic()
    : state(undef)
  {
  }

  std_logic(const bool& rhs)
  {
    operator=(rhs);
  }

  std_logic(const char& rhs)
  {
    operator=(rhs);
  }

  std_logic& operator=(const std_logic& rhs)
  {
    state = rhs.state;
    return *this;
  }

  std_logic& operator=(const bool& rhs)
  {
    state = (rhs ? high : low);
    return *this;
  }

  std_logic& operator=(const char& rhs)
  {
    switch(rhs)
      {
      case '1':
        state = high;
        break;
      case '0':
        state = low;
        break;
      case 'Z':
        state = highz;
        break;
      default:
        state = undef;
        break;
      }
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

  std_logic operator not() const
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
};

std::ostream& operator<<(std::ostream& os, const std_logic& rhs)
{
  os << (char)rhs;
  return os;
}

std_logic resolve(std::map<hdl::detail::process_base*, std_logic> candidates,
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
                << " has been updated by the following processes: ";
      for(auto &i : candidates)
        std::cerr << i.first->getname() << " ";
      std::cerr << std::endl;
      return std_logic('U');
      break;
    } 
}

#endif // STD_LOGIC_HPP
