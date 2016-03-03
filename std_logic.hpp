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

#ifndef STD_LOGIC_HPP
#define STD_LOGIC_HPP

class std_logic
{
private:
  enum state_t { high, low, highz, undef };
  state_t state;

  inline std_logic lut2(const std_logic &rhs, state_t hh, state_t hl, state_t lh, state_t ll) const
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

  std_logic z()
  {
    std_logic result;
    result.state = highz;
    return result;
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

  bool operator>(const std_logic& rhs) const
  {
    return lut2(rhs, low, high, low, low);
  }

  bool operator>=(const std_logic& rhs) const
  {
    return lut2(rhs, high, high, low, high);
  }

  bool operator<(const std_logic& rhs) const
  {
    return lut2(rhs, low, low, high, low);
  }

  bool operator<=(const std_logic& rhs) const
  {
    return lut2(rhs, high, low, high, high);
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

  std_logic operator +(const std_logic &rhs) const
  {
    return lut2(rhs, low, high, high, low);
  }

  std_logic operator -(const std_logic &rhs) const
  {
    return lut2(rhs, low, high, high, low);
  }

  std_logic operator *(const std_logic &rhs) const
  {
    return lut2(rhs, high, low, low, low);
  }

  std_logic operator /(const std_logic &rhs) const
  {
    return lut2(rhs, high, undef, low, undef);
  }

  std_logic operator %(const std_logic &rhs) const
  {
    return lut2(rhs, low, undef, low, undef);
  }

  std_logic operator +() const
  {
    return *this;
  }

  std_logic operator -() const
  {
    return operator not();
  }
};

std::ostream& operator<<(std::ostream& os, const std_logic& rhs)
{
  os << (char)rhs;
  return os;
}

#ifdef MULTIASSIGN
std_logic resolve(const std::map<hdl::detail::base*, std_logic> &candidates,
                  const hdl::detail::base *w)
{
  std_logic result = 'Z';
  unsigned int nonzcnt = 0;
  for(auto &i : candidates)
    if((char)i.second != 'Z')
      {
        nonzcnt++;
        if(nonzcnt == 1)
          result = i.second;
        else
          {
            std::cerr << "WARNING: wire " << w->getname()
                      << " has been updated by the following parts: ";
            for(auto &i : candidates)
              {
                if(i.first)
                  std::cerr << i.first->getname() << " ";
                else
                  std::cerr << "testbench ";
                std::cerr << "(" << i.second << ") ";
              }
            std::cerr << std::endl;
            result = 'U';
            break;
          }
      }
  return result;
}
#endif

#endif // STD_LOGIC_HPP
