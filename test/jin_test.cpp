#include <boost/test/unit_test.hpp>

#include <jios/json_in.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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

BOOST_AUTO_TEST_CASE( parse_time_test )
{
  istringstream ss(R"(["00:00:03", "00:00:02", "00:00:01"])");
  using namespace boost::posix_time;
  time_duration t3, t2, t1;
  json_in(ss).get().array() >> t3 >> t2 >> t1;
  BOOST_CHECK_EQUAL( t3, seconds(3) );
  BOOST_CHECK_EQUAL( t2, seconds(2) );
  BOOST_CHECK_EQUAL( t1, seconds(1) );
}

BOOST_AUTO_TEST_CASE( parse_bad_time_test )
{
  istringstream ss("\"00:00:03 trash\"");
  using namespace boost::posix_time;
  time_duration t;
  json_in(ss) >> t;
  BOOST_CHECK( ss.fail() );
}

