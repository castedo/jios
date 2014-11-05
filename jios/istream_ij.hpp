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

  char peek()
  {
    return (bytes_avail_ ? buf_[0] : p_is_->peek());
  }

  bool good() const { return !this->fail() && !this->eof(); }

  bool eof() const
  {
    BOOST_ASSERT(!bytes_avail_ || !p_is_->eof());
    return p_is_->eof();
  }

  const char * begin() const { return buf_.data(); }

  std::streamsize avail();

  void readsome_nonws();

  void remove(std::streamsize n);

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


} // namespace jios

#endif

