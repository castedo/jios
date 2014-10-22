#include <boost/test/unit_test.hpp>

#include <jios/json_in.hpp>

using namespace std;
using namespace jios;

BOOST_AUTO_TEST_CASE( simple_jin_test )
{
  istringstream ss("1 \"two\" 3");
  int i, j;
  string s;
  json_in(ss) >> i >> s >> j;
  BOOST_CHECK_EQUAL( i, 1 );
  BOOST_CHECK_EQUAL( s, "two" );
  BOOST_CHECK_EQUAL( j, 3 );
  BOOST_CHECK( ss.eof() );
}

