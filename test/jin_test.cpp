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

void test_empty_array(string json)
{
  istringstream ss(json);
  list<int> a;
  a.push_back(666);
  json_in(ss) >> a;
  BOOST_CHECK( a.empty() );

  ss.str(json);
  ijstream jin = json_in(ss);
  BOOST_CHECK( !jin.at_end() );
  ijvalue & ij = jin.get();
  BOOST_CHECK( ij.is_array() );
  ijarray ija = ij.array();
  BOOST_CHECK( ija.at_end() );
}

BOOST_AUTO_TEST_CASE( parse_empty_arrays )
{
  test_empty_array("[]");
  test_empty_array(" []");
  test_empty_array("[ ]");
  test_empty_array("[] ");
  test_empty_array(" [ ]");
  test_empty_array(" [] ");
  test_empty_array("[ ] ");
}

void test_array(string json, list<int> expect)
{
  istringstream ss(json);
  ijstream jin = json_in(ss);
  list<int> a;
  jin >> a;
  BOOST_CHECK_EQUAL( a.size(), expect.size() );
  BOOST_CHECK( a == expect );
  BOOST_CHECK( !ss.fail() );
  BOOST_CHECK( !ss.eof() );
  BOOST_CHECK( jin.at_end() );
  BOOST_CHECK( !ss.fail() );
  BOOST_CHECK( ss.eof() );
}

BOOST_AUTO_TEST_CASE( parse_lists_test )
{
  test_array("[1 ]", {1});
  test_array("[1 , 2 ]", {1, 2});
  test_array("[1, 2, 3]", {1, 2, 3});
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

BOOST_AUTO_TEST_CASE( simple_eof_test )
{
  stringstream ss("  \n     \n   ");
  ijstream jin = json_in(ss);
  BOOST_CHECK( jin.at_end() );
  BOOST_CHECK( ss.eof() );
}

BOOST_AUTO_TEST_CASE( simple_incremental_test )
{
  stringstream ss;
  int i;
  ijstream jin = json_in(ss);
  BOOST_CHECK( jin.expecting() );
  BOOST_CHECK( !ss.eof() );

  ss << "1 ";
  jin >> i;
  BOOST_CHECK_EQUAL( i, 1 );
  BOOST_CHECK( jin.expecting() );
  BOOST_CHECK( !ss.eof() );

  ss << "2 ";
  jin >> i;
  BOOST_CHECK_EQUAL( i, 2 );
  BOOST_CHECK( jin.expecting() );
  BOOST_CHECK( !ss.eof() );

  ss << "3";
  jin >> i;
  BOOST_CHECK_EQUAL( i, 3 );

  BOOST_CHECK( jin.at_end() );
  BOOST_CHECK( ss.eof() );
}

BOOST_AUTO_TEST_CASE( async_test )
{
  stringstream ss;
  int i;
  ijstream jin = json_in(ss);
  BOOST_CHECK( jin.expecting() );
  BOOST_CHECK( !ss.eof() );

  ss << "12 ";
  BOOST_REQUIRE_GT( ss.rdbuf()->in_avail(), 0 );
  BOOST_CHECK( !jin.expecting() );
  jin >> i;
  BOOST_CHECK_EQUAL( i, 12 );

  ss << " 345";
  BOOST_CHECK( jin.expecting() );
  BOOST_CHECK( !ss.eof() );
  ss << "67 ";
  BOOST_CHECK( !jin.expecting() );
  jin >> i;
  BOOST_CHECK_EQUAL( i, 34567 );
}

BOOST_AUTO_TEST_CASE( dont_skip_ws_test )
{
  stringstream ss;
  ijstream jin = json_in(ss);
  ss << '"' << " hello";
  BOOST_CHECK( jin.expecting() );
  ss << " world ";
  BOOST_CHECK( jin.expecting() );
  ss << '"';
  BOOST_CHECK( !jin.expecting() );

  string rez;
  jin >> rez;
  BOOST_CHECK_EQUAL( rez, " hello world " );
  BOOST_CHECK( !ss.fail() );
  BOOST_CHECK( !ss.eof() );
  BOOST_CHECK( jin.expecting() );
  BOOST_CHECK( !ss.eof() );
}

