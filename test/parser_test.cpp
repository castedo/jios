#include <boost/test/unit_test.hpp>

#include <jios/jsonc_parser.hpp>

using namespace std;
using namespace jios;

class simple_ijstate : public ijstate
{
  bool failed_;

  virtual bool do_get_failbit() const { return failed_; }
  virtual void do_set_failbit() { failed_ = true; }

public:
  simple_ijstate() : failed_(false) {}
};

BOOST_AUTO_TEST_CASE( parser_test )
{
  stringstream ss;
  ijstream ij(make_jsonc_parser(make_shared<istream_facade>(ss)));

  BOOST_CHECK( !ij.ready() );
  ss << "{}";
  BOOST_CHECK( ij.ready() );
  BOOST_CHECK( !ij.at_end() );
  BOOST_CHECK( ij.get().is_object() );

  ss <<"[";
  BOOST_CHECK( !ij.ready() );
  ss << "]";
  BOOST_CHECK( ij.ready() );
  BOOST_CHECK( !ij.at_end() );
  BOOST_CHECK( ij.get().is_array() );

  BOOST_CHECK(!ij.ready());
  BOOST_CHECK(ij.at_end());
}

