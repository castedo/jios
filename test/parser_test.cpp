#include <boost/test/unit_test.hpp>

#include <jios/jsonc_parser.hpp>

using namespace std;
using namespace jios;

BOOST_AUTO_TEST_CASE( parser_test )
{
  stringstream ss;
  ijstream ij(make_jsonc_ijsource(make_shared<istream_facade>(ss)));

  BOOST_CHECK( ij.expecting() );
  ss << "{}";
  BOOST_CHECK( !ij.expecting() );
  BOOST_CHECK( !ij.at_end() );
  BOOST_CHECK( ij.get().is_object() );

  ss <<"[";
  BOOST_CHECK( ij.expecting() );
  ss << "]";
  BOOST_CHECK( !ij.expecting() );
  BOOST_CHECK( !ij.at_end() );
  BOOST_CHECK( ij.get().is_array() );

  BOOST_CHECK(ij.expecting());
  BOOST_CHECK(ij.at_end());
}

BOOST_AUTO_TEST_CASE( peek_eof_test )
{
  {
    stringstream ss("\xFF ");
    BOOST_REQUIRE( !ss.eof() );
    BOOST_REQUIRE_EQUAL( ss.peek(), 0xFF );
    BOOST_REQUIRE_NE( ss.peek(), EOF );
    BOOST_REQUIRE_EQUAL( ss.get(), 0xFF );
    BOOST_REQUIRE( !ss.eof() );
    BOOST_REQUIRE_EQUAL( ss.get(), ' ' );
    BOOST_REQUIRE( !ss.eof() );
    BOOST_REQUIRE_EQUAL( ss.peek(), EOF );
    BOOST_REQUIRE( ss.eof() );
  }
  {
    istream_facade fac(make_shared<stringstream>("\xFF "));
    BOOST_CHECK( !fac.eof() );
    BOOST_CHECK_EQUAL( fac.peek(), 0xFF );
    BOOST_CHECK_NE( fac.peek(), EOF );
    BOOST_CHECK_GE( fac.avail(), 1 );
    BOOST_CHECK_EQUAL( fac.peek(), 0xFF );
    BOOST_CHECK_NE( fac.peek(), EOF );
    fac.remove(1);
    BOOST_CHECK( !fac.eof() );
    BOOST_CHECK_EQUAL( fac.peek(), ' ' );
    BOOST_CHECK_GE( fac.avail(), 1 );
    BOOST_CHECK( !fac.eof() );
    fac.remove(1);
    BOOST_CHECK_EQUAL( fac.peek(), EOF );
    BOOST_CHECK( fac.eof() );
  }
}

