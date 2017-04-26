#ifndef CEL_JIOS_JSON_OJ_HPP
#define CEL_JIOS_JSON_OJ_HPP

#include <memory>
#include <ostream>
#include <jios/jout.hpp>

namespace jios {


// ojstream simple implementations outputing JSON

ojstream json_out(std::ostream & os, char delim = EOF);
ojstream json_out(std::shared_ptr<std::ostream> const&, char delim = EOF);

ojstream lined_json_out(std::ostream & os);
ojstream lined_json_out(std::shared_ptr<std::ostream> const&,
                        char delim = EOF);

template<class T>
struct enable_stream_out
{
  friend
  std::ostream & operator << (std::ostream & os, T const& src)
  {
    jios_write(json_out(os).put(), src);
    return os;
  }
};


} // namespace

#endif

