#include <jios/istream_ij.hpp>

#include <boost/throw_exception.hpp>
#include <boost/core/null_deleter.hpp>

using namespace std;

namespace jios {


// istream_facade

istream_facade::istream_facade(shared_ptr<istream> const& p_is)
  : p_is_(p_is)
  , buf_(4096)
  , bytes_avail_(0)
{
  if (!p_is_) {
    BOOST_THROW_EXCEPTION(bad_alloc());
  }
}

istream_facade::istream_facade(istream & is)
  : p_is_(&is, boost::null_deleter())
  , buf_(4096)
  , bytes_avail_(0)
{
}

streamsize istream_facade::avail()
{
  if (!bytes_avail_ && p_is_->good()) {
    bytes_avail_ = p_is_->readsome(buf_.data(), buf_.size());
  }
  return bytes_avail_;
}

void istream_facade::readsome_nonws()
{
  istream & is = *p_is_;
  if (!bytes_avail_ && is.good()) {
    bytes_avail_ = is.readsome(buf_.data(), buf_.size());
  }
  char const* it = buf_.data();
  while (bytes_avail_ > 0 && ::isspace(*it)) {
    ++it;
    --bytes_avail_;
    if (!bytes_avail_ && is.good()) {
      bytes_avail_ = is.readsome(buf_.data(), buf_.size());
      it = buf_.data();
    }
  }
  if (bytes_avail_ > 0 && it != buf_.data()) {
    copy(it, it + bytes_avail_, buf_.data());
  }
}

void istream_facade::remove(streamsize n)
{
  BOOST_ASSERT( n <= bytes_avail_ );
  if (n > bytes_avail_) { n = bytes_avail_; }
  bytes_avail_ -= n;
  if (bytes_avail_ > 0) {
    char const* rest = buf_.data() + n;
    copy(rest, rest + bytes_avail_, buf_.data());
  }
}

shared_ptr<ijsource>
    make_array_ijsource(shared_ptr<istream_facade> const& p_is,
                        istream_ijsource_factory const& factory)
{
  return factory(p_is);
}


} // namespace

