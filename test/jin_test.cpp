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

BOOST_AUTO_TEST_CASE( jin_for_loop_test )
{
  istringstream ss("1 2 3 4 5");
  int sum = 0;
  for (ijvalue & v : json_in(ss)) {
    int i;
    if (v.read(i)) {
      sum += i;
    }
  }
  BOOST_CHECK( !ss.fail() );
  BOOST_CHECK_EQUAL( sum, 15 );
}

BOOST_AUTO_TEST_CASE( jin_for_loop_object_test )
{
  stringstream ss;
  ss << R"( { "a":1, "bc":2, "def":3 } )";
  int sum = 0;
  for (ijpair & nv : json_in(ss).get().object()) {
    int i;
    if (nv.read(i)) {
      sum += i;
    }
    BOOST_CHECK_EQUAL( nv.key().size(), i );
  }
  BOOST_CHECK( !ss.fail() );
  BOOST_CHECK_EQUAL( sum, 6 );
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

BOOST_AUTO_TEST_CASE( parse_list_test )
{
  istringstream ss("[1, 2, 3]");
  list<int> a;
  a.push_back(4);
  json_in(ss) >> a;
  list<int> expect = {1, 2, 3};
  BOOST_CHECK_EQUAL( a.size(), expect.size() );
  BOOST_CHECK( a == expect );
}

BOOST_AUTO_TEST_CASE( parse_tie_test )
{
  stringstream ss;
  ss << R"( { "Joe":35, "Jane":33 } )";

  string name1, name2;
  int age1, age2;

  json_in(ss).get().object() >> tie(name1, age1)
                             >> tie(name2, age2);

  BOOST_CHECK_EQUAL( name1, "Joe" ); 
  BOOST_CHECK_EQUAL( name2, "Jane" ); 
  BOOST_CHECK_EQUAL( age1, 35 ); 
  BOOST_CHECK_EQUAL( age2, 33 ); 

  stringstream out;
  out << name1 << " is " << age1 - age2 << " years older than "
      << name2 << endl;
  BOOST_CHECK_EQUAL( out.str(), "Joe is 2 years older than Jane\n");
}

BOOST_AUTO_TEST_CASE( parse_keys_as_int_test )
{
  stringstream ss;
  ss << R"( { "1":1, "2":4, "3":9 } )";

  ijobject ijo = json_in(ss).get().object();
  while (!ijo.at_end()) {
    int key, value;
    ijo >> tie(key, value);
    BOOST_CHECK_EQUAL( key*key, value );
  }
}

