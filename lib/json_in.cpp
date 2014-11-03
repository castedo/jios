#include <jios/json_in.hpp>

#include <boost/core/null_deleter.hpp>
#include <jios/jsonc_parser.hpp>

using namespace std;

namespace jios {


class istream_jin_state : public ijstate
{
public:
  istream_jin_state(shared_ptr<istream> const& p_is) : p_is_(p_is) {}

private:
  shared_ptr<istream> p_is_;

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
    : p_parser_(make_jsonc_parser(make_shared<istream_jin_state>(p_is)))
    , p_is_(p_is)
    , buf_(4096)
    , bytes_avail_(0)
  {
    if (!p_is_) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
    induce();
  }

private:
  ijstate & do_state() override { return p_parser_->state(); }
  ijstate const& do_state() const override { return p_parser_->state(); }

  ijpair & do_ref() override { return p_parser_->dereference(); }

  bool do_is_terminator() override;
  void do_advance() override;
  bool do_ready() override;
  void induce();

  shared_ptr<ijsource_parser> p_parser_;
  shared_ptr<istream> p_is_;
  vector<char> buf_;
  streamsize bytes_avail_;
};

bool jsonc_root_ijnode::do_is_terminator()
{
  return p_parser_->at_terminator();
}

void jsonc_root_ijnode::do_advance()
{
  p_parser_->advance();
  if (!this->fail()) { induce(); }
}

void readsome_until_nonws(istream & is, vector<char> & buf, streamsize & count)
{
  if (!count && is.good()) {
    count = is.readsome(buf.data(), buf.size());
  }
  char const* it = buf.data();
  while (count > 0 && ::isspace(*it)) {
    ++it;
    --count;
    if (!count && is.good()) {
      count = is.readsome(buf.data(), buf.size());
      it = buf.data();
    }
  }
  if (count > 0 && it != buf.data()) {
    copy(it, it + count, buf.data());
  }
}

bool jsonc_root_ijnode::do_ready()
{
  BOOST_ASSERT(p_is_);
  if (!p_is_) return true;
  if (this->fail()) return true;
  istream & is = *p_is_;
  if (!p_parser_->ready()) {
    readsome_until_nonws(is, buf_, bytes_avail_);
    while (bytes_avail_ > 0 && !p_parser_->ready() && !this->fail()) {
      streamsize parsed = p_parser_->parse_some(buf_.data(), bytes_avail_);
      if (!this->fail()) {
        bytes_avail_ -= parsed;
        if (bytes_avail_ > 0) {
          char const* rest = buf_.data() + parsed;
          copy(rest, rest + bytes_avail_, buf_.data());
        } else {
          bytes_avail_ = is.readsome(buf_.data(), buf_.size());
        }
      }
    }
    if (p_parser_->ready()) {
      readsome_until_nonws(is, buf_, bytes_avail_);
    }
  }
  return !is.good() || p_parser_->ready();
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

