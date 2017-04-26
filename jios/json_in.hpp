#ifndef JIOS_JSON_IN_HPP
#define JIOS_JSON_IN_HPP

#include <memory>
#include <istream>
#include <boost/optional.hpp>

#include <jios/jin.hpp>

namespace jios {


ijstream json_in(std::istream & is);
ijstream json_in(std::shared_ptr<std::istream> const& p_is);


template<class T>
struct enable_stream_in
{
  friend
  std::istream & operator >> (std::istream & is, T & dest)
  {
    boost::optional<T> o_dest;
    jios::ijstream jin = jios::json_in(is);
    jios_read(jin.get(), o_dest);
    if (!is.fail() && o_dest) { dest = *o_dest; }
    return is;
  }

  friend
  std::istream & operator >> (std::istream & is, boost::optional<T> & o_dest)
  {
    jios::ijstream jin = jios::json_in(is);
    jios_read(jin.get(), o_dest);
    return is;
  }
};


}

#endif

