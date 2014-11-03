#ifndef JIOS_JSONC_PARSER_HPP
#define JIOS_JSONC_PARSER_HPP

#include <memory>
#include <streambuf>
#include <jios/jin.hpp>

namespace jios {


class ijsource_parser : public ijsource
{
public:
  virtual ~ijsource_parser() {}

  ijstate & state() { return do_state(); }
  ijstate const& state() const { return do_state(); }
  ijpair & dereference() { return do_ref(); }
  bool at_terminator() { return do_is_terminator(); }
  void advance() { do_advance(); }
  bool ready() { return do_ready(); }

  std::streamsize parse_some(const char* p, std::streamsize n)
  {
    std::streamsize ret = do_parse_some(p, n);
    BOOST_ASSERT(ret > 0);
    if (ret <= 0) {
      do_state().set_failbit();
      return 0;
    }
    return ret;
  }

private:
  virtual std::streamsize do_parse_some(const char* p, std::streamsize n) = 0;
};


// factory function

std::shared_ptr<ijsource_parser>
    make_jsonc_parser(std::shared_ptr<ijstate> const&);


} // namespace jios

#endif

