#ifndef JIOS_ISTREAM_IJ_HPP
#define JIOS_ISTREAM_IJ_HPP

#include <memory>
#include <vector>
#include <istream>
#include <jios/jin.hpp>

namespace jios {


class istream_facade : public ijstate
{
public:
  istream_facade(std::shared_ptr<std::istream> const& p_is);

  void peek() { p_is_->peek(); }

  bool good() { return p_is_->good(); }

  const char * begin() { return buf_.data(); }

  std::streamsize avail() const { return bytes_avail_; }

  void readsome_nonws();

  void readsome_if_empty();

  void extract(std::streamsize n);

private:
  std::shared_ptr<std::istream> p_is_;
  std::vector<char> buf_;
  std::streamsize bytes_avail_;

  bool do_get_failbit() const override
  {
    BOOST_ASSERT(p_is_);
    return (p_is_ ? p_is_->fail() : true);
  }

  void do_set_failbit() override
  {
    BOOST_ASSERT(p_is_);
    if (p_is_) {
      p_is_->setstate(std::ios_base::failbit);
    }
  }
};

class ijsource_parser : public ijsource
{
public:
  virtual ~ijsource_parser() {}

  ijstate & state() { return do_state(); }
  ijstate const& state() const { return do_state(); }
  ijpair & dereference() { return do_ref(); }
  bool at_terminator() { return do_is_terminator(); }
  bool hint_multiline() { return do_hint_multiline(); }
  void advance() { do_advance(); }
  bool ready() { return do_ready(); }

  std::streamsize parse_some(const char* p, std::streamsize n)
  {
    std::streamsize ret = do_parse_some(p, n);
    if (ret <= 0) {
      BOOST_ASSERT(do_state().fail());
      do_state().set_failbit();
      return 0;
    }
    return ret;
  }

private:
  virtual std::streamsize do_parse_some(const char* p, std::streamsize n) = 0;
};


} // namespace jios

#endif

