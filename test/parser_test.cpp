#include <boost/test/unit_test.hpp>

#include <jios/jsonc_parser.hpp>
#include <jios/json_in.hpp>

using namespace std;
using namespace jios;

BOOST_AUTO_TEST_CASE( incremental_array_parse)
{
  stringstream ss;
  ijstream jin = json_in(ss);
  int i = 0;

  BOOST_CHECK( jin.expecting() );
  ss << "1 ";
  BOOST_CHECK( !jin.expecting() );
  jin >> i;
  BOOST_CHECK_EQUAL( i, 1 );

  ss << "[";
  BOOST_CHECK( !jin.expecting() );
  ijarray ija = jin.get().array();
  BOOST_CHECK( ija.expecting() );
  
  ss << " 2 ";
  BOOST_CHECK( !ija.expecting() );
  BOOST_CHECK_EQUAL( ija.peek().type(), json_type::jinteger );
  ija >> i;
  BOOST_CHECK_EQUAL( i, 2 );
  BOOST_CHECK( ija.expecting() );
  ss << ",";
  BOOST_CHECK( ija.expecting() );

  ss << " 3 ";
  BOOST_CHECK( !ija.expecting() );
  ija >> i;
  BOOST_CHECK_EQUAL( i, 3 );
  BOOST_CHECK( ija.expecting() );

  ss << " ]";
  BOOST_CHECK( !ija.expecting() );
  BOOST_CHECK( ija.at_end() );
  BOOST_CHECK( !ija.fail() );
}

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

