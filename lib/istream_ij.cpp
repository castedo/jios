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
  //! Return to initial start state if no failure
  //! If not at end, read/parse until end.
  virtual void do_restart() = 0;

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

  //! Advance until reaching terminator, if no failure.
  //! Then enter initial start state as though nothing has been read.
  void restart() { do_restart(); }

protected:
  ijstate & do_state() override { return *p_is_; }
  ijstate const& do_state() const override { return *p_is_; }
  ijpair & do_ref() override { induce(); return p_parser_->result(); }

  void induce();

  shared_ptr<istream_facade> p_is_;
  shared_ptr<istream_parser> p_parser_;
};

void istream_ijsource::induce()
{
  while (this->expecting()) {
    p_is_->peek();
  }
}

// istream_stream_ijsource

class istream_stream_ijsource : public istream_ijsource
{
public:
  istream_stream_ijsource(shared_ptr<istream_facade> const& p_is,
                          shared_ptr<istream_parser> const& p_p)
    : istream_ijsource(p_is, p_p)
  {}

private:
  bool do_is_terminator() override;
  void do_advance() override;
  bool do_expecting() override;
  void do_restart() override;
};

bool istream_stream_ijsource::do_is_terminator()
{
  induce();
  return !p_parser_->is_parsed();
}

void istream_stream_ijsource::do_advance()
{
  induce();
  p_parser_->clear();
}

bool istream_stream_ijsource::do_expecting()
{
  p_parser_->parse(p_is_);
  return !p_parser_->is_parsed() && p_is_->good();
}

void istream_stream_ijsource::do_restart()
{
  p_parser_->clear();
}

shared_ptr<ijsource>
    make_stream_ijsource(shared_ptr<istream_facade> const& p_is,
                         shared_ptr<istream_parser> const& p_p)
{
  return make_shared<istream_stream_ijsource>(p_is, p_p);
}

// istream_array_ijsource

class istream_array_ijsource : public istream_ijsource
{
public:
  istream_array_ijsource(shared_ptr<istream_facade> const& p_is,
                         shared_ptr<istream_parser> const& p_p)
    : istream_ijsource(p_is, p_p)
    , state_(parse_state::start)
    , multiline_(false)
  {}


private:
  bool do_is_terminator() override;
  void do_advance() override;
  bool do_expecting() override;
  bool do_hint_multiline() const override { return multiline_; }
  void do_restart() override;

  bool parse_char();

  enum class parse_state {
    start,
    poststart,
    value,
    predelim,
    finish
  };

  parse_state state_;
  bool multiline_;
};

bool istream_array_ijsource::do_is_terminator()
{
  induce();
  return state_ == parse_state::finish || this->fail();
}

void istream_array_ijsource::do_advance()
{
  induce();
  BOOST_ASSERT(state_ != parse_state::finish);
  BOOST_ASSERT(state_ == parse_state::value || this->fail());
  p_parser_->clear();
  if (state_ == parse_state::value) {
    state_ = parse_state::predelim;
  }
}

bool istream_array_ijsource::parse_char()
{
  BOOST_ASSERT(p_is_->avail());
  char ch = *(p_is_->begin());
  switch (state_) {
    case parse_state::start:
      if (::isspace(ch)) return true;
      if (ch == '[') {
        if (p_is_->avail()) {
          ch = *(p_is_->begin());
          multiline_ = (ch == '\n');
        }
        state_ = parse_state::poststart;
        return true;
      }
      break;
    case parse_state::poststart:
      if (::isspace(ch)) return true;
      if (ch == ']') {
        state_ = parse_state::finish;
        return true;
      } else {
        state_ = parse_state::value;
        return false;
      }
      break;
    case parse_state::value:
      return false;
    case parse_state::predelim:
      if (::isspace(ch)) return true;
      if (ch == ']') {
        state_ = parse_state::finish;
        return true;
      } else if (ch == ',') {
        state_ = parse_state::value;
        return true;
      }
      break;
    case parse_state::finish:
      return false;
  }
  this->set_failbit();
  return false;
}

bool istream_array_ijsource::do_expecting()
{
  while (p_is_->avail() && parse_char()) {
    p_is_->remove(1);
  }
  if (p_is_->eof()) {
    this->set_failbit();
  }
  switch (state_) {
    case parse_state::value:
      p_parser_->parse(p_is_);
      return !p_parser_->is_parsed() && p_is_->good();
    case parse_state::finish:
      return false;
    default:
      break;
  }
  return p_is_->good();
}

void istream_array_ijsource::do_restart()
{
  if (state_ != parse_state::start) {
    while (!this->is_terminator() && !this->fail()) {
      this->advance();
    }
  }
  p_parser_->clear();
  state_ = parse_state::start;
  multiline_ = false;
}

// streaming_parser

class streaming_parser : public istream_parser, private ijpair
{
public:
  streaming_parser(shared_ptr<istream_facade> const& p_is,
                   istream_parser_factory const& fallback)
    : p_is_(p_is)
    , p_parser_(fallback(p_is))
  {
    if (!p_is_ || !p_parser_) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
  }

private:
  // istream_parser virtual methods
  void do_clear() override;
  void do_parse(std::shared_ptr<istream_facade> const& p_is) override {}
  bool do_is_parsed() const override { return true; }
  ijpair & do_result() { return *this; }

  // ijpair virtual methods
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
  shared_ptr<istream_parser> p_parser_;
  shared_ptr<istream_ijsource> p_src_;
};

void streaming_parser::do_clear()
{
  p_src_->restart();
  p_parser_->clear();
}

ijarray streaming_parser::do_begin_array()
{
  if (p_src_) {
    p_src_->restart();
  } else {
    p_src_.reset(new istream_array_ijsource(p_is_, p_parser_));
  }
  return ijarray(p_src_);
}

shared_ptr<istream_parser>
    make_streaming_parser(shared_ptr<istream_facade> const& p_is,
                          istream_parser_factory const& fallback)
{
  return make_shared<streaming_parser>(p_is, fallback);
}


} // namespace

