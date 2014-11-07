#include <jios/json_in.hpp>

#include <boost/core/null_deleter.hpp>
#include <jios/jsonc_parser.hpp>

using namespace std;

namespace jios {


class split_parser : public istream_parser
{
public:
  split_parser(shared_ptr<istream_facade> const& p_is)
    : choice_done_(false)
    , use_alt_(false)
  {
  }

private:
  void do_clear() override;
  void do_parse(shared_ptr<istream_facade> const&) override;
  bool do_is_parsed() const override;
  ijpair & do_result() override;

  istream_parser * try_choice() const;

  shared_ptr<istream_parser> p_default_;
  shared_ptr<istream_parser> p_alt_;
  bool choice_done_;
  bool use_alt_;
};

shared_ptr<istream_parser>
    make_split_parser(shared_ptr<istream_facade> const& p_is)
{
  return shared_ptr<istream_parser>(new split_parser(p_is));
}

istream_parser * split_parser::try_choice() const
{
  if (!choice_done_) return nullptr;
  istream_parser * ret = (use_alt_  ? p_alt_.get() : p_default_.get());
  BOOST_ASSERT(ret);
  return ret;
}

void split_parser::do_clear()
{
  choice_done_ = false;
  use_alt_ = false;
  if (p_default_) { p_default_->clear(); } 
  if (p_alt_) { p_alt_->clear(); } 
}

ijpair & split_parser::do_result()
{
  istream_parser * p = try_choice();
  if (!p) { BOOST_THROW_EXCEPTION(logic_error("result called when nothing parsed")); }
  return p->result();
}

bool split_parser::do_is_parsed() const
{
  istream_parser * p = try_choice();
  return (p ? p->is_parsed() : false);
}

void split_parser::do_parse(shared_ptr<istream_facade> const& p_in)
{
  istream_facade & in = *p_in;
  if (!choice_done_) {
    in.eat_whitespace();
    if (in.avail()) {
      char next_nonws = *(in.begin());
      use_alt_ = (next_nonws == '[');
      choice_done_ = true;
    }
  }
  if (choice_done_) {
    if (use_alt_) {
      if (!p_alt_) {
        istream_parser_factory fallback = &make_split_parser;
        p_alt_ = make_streaming_parser(p_in, fallback);
        if (!p_alt_) { BOOST_THROW_EXCEPTION(bad_alloc()); }
      }
      p_alt_->parse(p_in);
    } else {
      if (!p_default_) {
        p_default_ = make_jsonc_parser(p_in);
        if (!p_default_) { BOOST_THROW_EXCEPTION(bad_alloc()); }
      }
      p_default_->parse(p_in);
    }
  }
}

// factory functions

ijstream json_in(shared_ptr<istream> const& p_is)
{
  shared_ptr<istream_facade> p_f(new istream_facade(p_is));
  return make_stream_ijsource(p_f, make_split_parser(p_f));
}

ijstream json_in(istream & is)
{
  return json_in(shared_ptr<istream>(&is, boost::null_deleter()));
}


} // namespace

