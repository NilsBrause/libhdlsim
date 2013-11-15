#ifndef STD_LOGIC_HPP
#define STD_LOGIC_HPP

class std_logic;
std::ostream& operator<<(std::ostream& os, const std_logic& rhs);

class std_logic
{
private:
  enum state_t { high, low, undef };
  state_t state;
      
  friend std::ostream& operator<<(std::ostream& os, const std_logic& rhs);

public:
  std_logic()
    : state(undef)
  {
  }

  std_logic(const bool& rhs)
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

  bool operator==(const std_logic& rhs)
  {
    return state == rhs.state;
  }

  bool operator!=(const std_logic& rhs)
  {
    return state != rhs.state;
  }

  operator bool()
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

  std_logic operator not()
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
  switch(rhs.state)
    {
    case std_logic::high:
      os << '1';
      break;
    case std_logic::low:
      os << '0';
      break;
    default:
      os << 'U';
      break;
    }
  return os;
}

#endif // STD_LOGIC_HPP
