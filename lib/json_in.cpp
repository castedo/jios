#include <jios/json_in.hpp>

#include <boost/core/null_deleter.hpp>
#include <jios/jsonc_parser.hpp>

using namespace std;

namespace jios {


class split_ijsource : public ijsource
{
public:
  split_ijsource(shared_ptr<istream> const& p_is)
    : p_in_(new istream_facade(p_is))
    , choice_done_(false)
    , use_alt_(false)
  {
  }

private:
  ijstate & do_state() override { return *p_in_; }
  ijstate const& do_state() const override { return *p_in_; }

  ijpair & do_ref() override;
  bool do_is_terminator() override;
  void do_advance() override;
  bool do_expecting() override;

  bool chosen();
  ijsource & choice();

  shared_ptr<istream_facade> p_in_;
  shared_ptr<ijsource> p_default_;
  shared_ptr<ijsource> p_alt_;
  bool choice_done_;
  bool use_alt_;
};

ijpair & split_ijsource::do_ref()
{
  return this->choice().dereference();
}

bool split_ijsource::do_is_terminator()
{
  return this->choice().is_terminator();
}

void split_ijsource::do_advance()
{
  this->choice().advance();
  choice_done_ = false;
  use_alt_ = false;
}

bool split_ijsource::do_expecting()
{
  if (!p_in_->good()) return false;
  if (!chosen()) return true;
  return this->choice().expecting();
}

bool split_ijsource::chosen()
{
  if (!choice_done_) {
    p_in_->readsome_nonws();
    streamsize num = p_in_->avail();
    if (num > 0) {
      char next_nonws = *(p_in_->begin());
      use_alt_ = (next_nonws == '[');
      choice_done_ = true;
    }
  }
  return choice_done_;
}

ijsource & split_ijsource::choice()
{
  while (!chosen() && p_in_->good()) {
    p_in_->peek();
  }
  if (use_alt_) {
    if (!p_alt_) {
      p_alt_ = make_jsonc_parser(p_in_);
      if (!p_alt_) { BOOST_THROW_EXCEPTION(bad_alloc()); }
    }
    return *p_alt_;
  } else {
    if (!p_default_) {
      p_default_ = make_jsonc_parser(p_in_);
      if (!p_default_) { BOOST_THROW_EXCEPTION(bad_alloc()); }
    }
    return *p_default_;
  }
}

// factory functions

ijstream json_in(shared_ptr<istream> const& p_is)
{
  return shared_ptr<ijsource>(new split_ijsource(p_is));
}

ijstream json_in(istream & is)
{
  return json_in(shared_ptr<istream>(&is, boost::null_deleter()));
}


} // namespace

