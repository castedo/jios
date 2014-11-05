#include <jios/json_in.hpp>

#include <boost/core/null_deleter.hpp>
#include <jios/jsonc_parser.hpp>

using namespace std;

namespace jios {


class jsonc_root_ijnode : public ijsource
{
public:
  jsonc_root_ijnode(shared_ptr<istream> const& p_is)
    : p_parser_(make_jsonc_parser(make_shared<istream_facade>(p_is)))
  {
    if (!p_parser_) {
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

  shared_ptr<ijsource> p_parser_;
};

ijpair & jsonc_root_ijnode::do_ref()
{
  return p_parser_->dereference();
}

bool jsonc_root_ijnode::do_is_terminator()
{
  return p_parser_->is_terminator();
}

void jsonc_root_ijnode::do_advance()
{
  p_parser_->advance();
}

bool jsonc_root_ijnode::do_ready()
{
  return p_parser_->ready();
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

