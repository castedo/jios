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

void istream_facade::eat_whitespace()
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

void istream_facade::remove_until(const char * it)
{
  remove(it - buf_.data());
}

// istream_ijsource

class istream_ijsource : public ijsource
{
public:
  istream_ijsource(shared_ptr<istream_facade> const& p_is,
                   shared_ptr<istream_parser> const& p_p)
    : p_is_(p_is)
    , p_parser_(p_p)
  {
    if (!p_is_ || !p_parser_) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
  }

private:
  void induce();

  ijstate & do_state() override { return *p_is_; }
  ijstate const& do_state() const override { return *p_is_; }

  ijpair & do_ref() override { induce(); return p_parser_->result(); }

  bool do_is_terminator() override { induce(); return !p_parser_->is_parsed(); }

  void do_advance() override { induce(); p_parser_->clear(); }

  bool do_expecting() override;

  shared_ptr<istream_facade> p_is_;
  shared_ptr<istream_parser> p_parser_;
};

void istream_ijsource::induce()
{
  while (this->expecting()) {
    p_is_->peek();
  }
}

bool istream_ijsource::do_expecting()
{
  p_parser_->parse(p_is_);
  return !p_parser_->is_parsed() && p_is_->good();
}

shared_ptr<ijsource>
    make_stream_ijsource(shared_ptr<istream_facade> const& p_is,
                         shared_ptr<istream_parser> const& p_p)
{
  return make_shared<istream_ijsource>(p_is, p_p);
}

// streaming_node 

class streaming_node : public ijpair
{
public:
  streaming_node(shared_ptr<istream_facade> const& p_is,
                 shared_ptr<istream_parser> const& p_p)
    : p_is_(p_is)
    , p_parser_(p_p)
  {
    if (!p_is_ || !p_parser_) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
  }

private:
  ijstate & do_state() override { return *p_is_; }
  ijstate const& do_state() const override { return *p_is_; }

  json_type do_type() const override { return json_type::jarray; }

  void do_parse(int64_t &) override { failout(); }
  void do_parse(double &) override { failout(); }
  void do_parse(bool &) override { failout(); }
  void do_parse(std::string &) override { failout(); }
  void do_parse(buffer_iterator) override { failout(); }

  ijarray do_begin_array() override;
  ijobject do_begin_object() { failout(); return ijobject(); }

  std::string do_key() const { BOOST_ASSERT(false); return ""; }

  void failout() { this->set_failbit(); }

  shared_ptr<istream_facade> p_is_;
public:
  shared_ptr<istream_parser> p_parser_;
};

ijarray streaming_node::do_begin_array()
{
  return p_parser_->result().array();
}

// streaming_parser

class streaming_parser : public istream_parser
{
public:
    streaming_parser(shared_ptr<istream_facade> const& p_is,
                     istream_parser_factory const& fallback)
      : node_(p_is, fallback(p_is))
    {}

private:
  void do_clear() override;
  void do_parse(std::shared_ptr<istream_facade> const& p_is) override;
  bool do_is_parsed() const override;
  ijpair & do_result() override;

  streaming_node node_;
};

void streaming_parser::do_clear()
{
  node_.p_parser_->clear();
}

void streaming_parser::do_parse(std::shared_ptr<istream_facade> const& p_is)
{
  node_.p_parser_->parse(p_is);
}

bool streaming_parser::do_is_parsed() const
{
  return node_.p_parser_->is_parsed();
}

ijpair & streaming_parser::do_result()
{
  return node_;
}

shared_ptr<istream_parser>
    make_streaming_parser(shared_ptr<istream_facade> const& p_is,
                          istream_parser_factory const& fallback)
{
  return make_shared<streaming_parser>(p_is, fallback);
}


} // namespace

