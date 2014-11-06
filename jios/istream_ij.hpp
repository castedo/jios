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

typedef std::function<
            std::shared_ptr<ijsource>(std::shared_ptr<istream_facade> const&)
        > istream_ijsource_factory;

std::shared_ptr<ijsource>
    make_array_ijsource(std::shared_ptr<istream_facade> const&,
                        istream_ijsource_factory const&);


} // namespace jios

#endif

