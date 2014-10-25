#include <boost/test/unit_test.hpp>

#include <jios/json_out.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace jios;

// streamed

BOOST_AUTO_TEST_CASE( simple_stream_test )
{
  ostringstream ss;
  json_out(ss, '\n') << 1 << 2 << "three";
  BOOST_CHECK_EQUAL( ss.str(), "1\n2\n\"three\"\n" );
}

BOOST_AUTO_TEST_CASE( simple_array_test )
{
  ostringstream ss;
  ojarray oja = json_out(ss).put().array();
  for (int i = 0; i < 3; ++i) {
    oja << i;
  }
  oja.terminate();     
  BOOST_CHECK_EQUAL( ss.str(), "[0, 1, 2]" );
}

BOOST_AUTO_TEST_CASE( simple_endj_test )
{
  ostringstream ss;
  json_out(ss).put().array() << 1 << 2 << 3 << endj;
  BOOST_CHECK_EQUAL( ss.str(), "[1, 2, 3]" );
}

BOOST_AUTO_TEST_CASE( simple_object_test )
{
  ostringstream ss;
  ojobject ojo = json_out(ss).put().object();
  string b = "BEE";
  ojo << make_pair("one", 1) << tie("two", b);
  ojo.terminate();     
  BOOST_CHECK_EQUAL( ss.str(), R"({"one":1, "two":"BEE"})" );
}

BOOST_AUTO_TEST_CASE( ostreamable_type_test )
{
  using namespace boost::posix_time;
  time_duration sleep = hours(7) + seconds(321);
  ostringstream ss;
  json_out(ss) << sleep;
  BOOST_CHECK_EQUAL( ss.str(), R"("07:05:21")" );

  ss.clear(); ss.str("");
  ojstream jout = json_out(ss);
  jout.put().array() << seconds(3) << seconds(2) << seconds(1) << endj;
  BOOST_CHECK_EQUAL( ss.str(), R"(["00:00:03", "00:00:02", "00:00:01"])" );

  ss.clear(); ss.str("");
  jout.put().object() << make_pair(seconds(3), "launch")
                      << make_pair(seconds(7), "explode") << endj;
  BOOST_CHECK_EQUAL( ss.str(), R"({"00:00:03":"launch", "00:00:07":"explode"})" );

}

BOOST_AUTO_TEST_CASE( empty_object_test )
{
  ostringstream ss;
  ojstream oj = json_out(ss);
  BOOST_CHECK_EQUAL( ss.str(), "" );
  ojobject ojo = oj.put().object();
  BOOST_CHECK_EQUAL( ss.str(), "{" );
  ojo.terminate();
  BOOST_CHECK_EQUAL( ss.str(), "{}" );
}

BOOST_AUTO_TEST_CASE( empty_array_test )
{
  ostringstream ss;
  ojstream oj = json_out(ss);
  BOOST_CHECK_EQUAL( ss.str(), "" );
  ojarray oja = oj.put().array();
  BOOST_CHECK_EQUAL( ss.str(), "[" );
  oja.terminate();
  BOOST_CHECK_EQUAL( ss.str(), "[]" );
}

BOOST_AUTO_TEST_CASE( lined_json_test )
{
  ostringstream ss;
  ojobject ojo = lined_json_out(ss).put().object(true);
  ojarray oja = ojo.put("A").array(true);
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[" );
  oja << "B";
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[\"B\"" );
  oja << "C";
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[\"B\",\"C\"" );
  oja.terminate();
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[\"B\",\"C\"]" );
  ojo.terminate();
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[\"B\",\"C\"]}\n" );
}

