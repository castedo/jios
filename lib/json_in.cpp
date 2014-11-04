#include <jios/json_in.hpp>

#include <boost/core/null_deleter.hpp>
#include <jios/jsonc_parser.hpp>

using namespace std;

namespace jios {


class istream_facade : public ijstate
{
public:
  istream_facade(shared_ptr<istream> const& p_is)
    : p_is_(p_is)
    , buf_(4096)
    , bytes_avail_(0)
  {
    if (!p_is_) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
  }

  void peek() { p_is_->peek(); }

  bool good() { return p_is_->good(); }

  const char * begin() { return buf_.data(); }

  streamsize avail() const { return bytes_avail_; }

  void readsome_nonws();

  void readsome_if_empty();

  void extract(streamsize n);

private:
  shared_ptr<istream> p_is_;
  vector<char> buf_;
  streamsize bytes_avail_;

  bool do_get_failbit() const override
  {
    BOOST_ASSERT(p_is_);
    return (p_is_ ? p_is_->fail() : true);
  }

  void do_set_failbit() override
  {
    BOOST_ASSERT(p_is_);
    if (p_is_) {
      p_is_->setstate(ios_base::failbit);
    }
  }
};

class jsonc_root_ijnode : public ijsource
{
public:
  jsonc_root_ijnode(shared_ptr<istream> const& p_is)
    : p_parser_(make_jsonc_parser(make_shared<istream_facade>(p_is)))
    , p_is_(new istream_facade(p_is))
  {
    if (!p_is_) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
  }

private:
  ijstate & do_state() override { return p_parser_->state(); }
  ijstate const& do_state() const override { return p_parser_->state(); }

  ijpair & do_ref() override;
  bool do_is_terminator() override;
  void do_advance() override;
  bool do_ready() override;
  void induce();

  shared_ptr<ijsource_parser> p_parser_;
  shared_ptr<istream_facade> p_is_;
};

ijpair & jsonc_root_ijnode::do_ref()
{
  induce();
  return p_parser_->dereference();
}

bool jsonc_root_ijnode::do_is_terminator()
{
  induce();
  return p_parser_->at_terminator();
}

void jsonc_root_ijnode::do_advance()
{
  induce();
  p_parser_->advance();
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

void istream_facade::readsome_if_empty()
{
  if (!bytes_avail_) {
    bytes_avail_ = p_is_->readsome(buf_.data(), buf_.size());
  }
}

void istream_facade::extract(streamsize n)
{
  bytes_avail_ -= n;
  if (bytes_avail_ > 0) {
    char const* rest = buf_.data() + n;
    copy(rest, rest + bytes_avail_, buf_.data());
  }
}

bool jsonc_root_ijnode::do_ready()
{
  BOOST_ASSERT(p_is_);
  if (!p_is_) return true;
  if (this->fail()) return true;
  if (!p_parser_->ready()) {
    //BUG: can skip whitespace inside string
    p_is_->readsome_nonws();
    while (p_is_->avail() > 0 && !p_parser_->ready() && !this->fail()) {
      streamsize parsed = p_parser_->parse_some(p_is_->begin(), p_is_->avail());
      if (!this->fail()) {
        p_is_->extract(parsed);
        p_is_->readsome_if_empty();  
      }
    }
    if (p_parser_->ready()) {
      p_is_->readsome_nonws();
    }
  }
  return !p_is_->good() || p_parser_->ready();
}

void jsonc_root_ijnode::induce()
{
  while (!this->do_ready()) {
    p_is_->peek();
  }
}

// factory functions

ijstream json_in(shared_ptr<istream> const& p_is)
{
  return shared_ptr<ijsource>(new jsonc_root_ijnode(p_is));
}

ijstream json_in(istream & is)
{
  return json_in(shared_ptr<istream>(&is, boost::null_deleter()));
}


} // namespace

