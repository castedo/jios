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
  istream_facade(std::istream & is);

  int peek()
  {
    return (bytes_avail_ ? int((unsigned char)buf_[0]) : p_is_->peek());
  }

  bool good() const { return !this->fail() && !this->eof(); }

  bool eof() const
  {
    BOOST_ASSERT(!bytes_avail_ || !p_is_->eof());
    return p_is_->eof();
  }

  const char * begin() const { return buf_.data(); }

  std::streamsize avail();

  void eat_whitespace();

  void remove(std::streamsize n);
  void remove_until(const char * it);

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

//! streaming JSON parser from istream source 


//! istream_parser
//! Interface to classes that can repeat parsing of JSON values using
//! istream_facade without any memory or context around those JSON values.

class istream_parser
{
  virtual void do_clear() = 0;
  virtual void do_parse(std::shared_ptr<istream_facade> const& p_is) = 0;
  virtual bool do_is_parsed() const = 0;
  virtual ijpair & do_result() = 0;

public:
  void clear() { do_clear(); }
  void parse(std::shared_ptr<istream_facade> const& p_is) { do_parse(p_is); }
  bool is_parsed() const { return do_is_parsed(); }
  ijpair & result() { return do_result(); }
};

typedef std::function<
    std::shared_ptr<istream_parser>(std::shared_ptr<istream_facade> const&)
> istream_parser_factory;

std::shared_ptr<ijsource>
    make_stream_ijsource(std::shared_ptr<istream_facade> const&,
                         std::shared_ptr<istream_parser> const&);

std::shared_ptr<istream_parser>
    make_streaming_parser(std::shared_ptr<istream_facade> const&,
                          istream_parser_factory const&);


} // namespace jios

#endif

