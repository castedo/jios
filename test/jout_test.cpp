#include <boost/test/unit_test.hpp>

#include <jios/json_out.hpp>

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

BOOST_AUTO_TEST_CASE( simple_object_test )
{
  ostringstream ss;
  ojobject ojo = json_out(ss).put().object();
  int a = 1 * 1;
  string b = "BEE";
  ojo << tie("one", a) << tie("two", b);
  ojo.terminate();     
  BOOST_CHECK_EQUAL( ss.str(), R"({"one":1, "two":"BEE"})" );
}

BOOST_AUTO_TEST_CASE( empty_object_test )
{
  ostringstream ss;
  ojstream oj = json_out(ss);
  BOOST_CHECK_EQUAL( ss.str(), "" );
  ojobject ojo = oj.put().begin_object();
  BOOST_CHECK_EQUAL( ss.str(), "{" );
  ojo.terminate();
  BOOST_CHECK_EQUAL( ss.str(), "{}" );
}

BOOST_AUTO_TEST_CASE( empty_array_test )
{
  ostringstream ss;
  ojstream oj = json_out(ss);
  BOOST_CHECK_EQUAL( ss.str(), "" );
  ojarray oja = oj.put().begin_array();
  BOOST_CHECK_EQUAL( ss.str(), "[" );
  oja.terminate();
  BOOST_CHECK_EQUAL( ss.str(), "[]" );
}

BOOST_AUTO_TEST_CASE( lined_json_test )
{
  ostringstream ss;
  ojobject ojo = lined_json_out(ss).put().begin_object(true);
  ojarray oja = ojo.put("A").array(true);
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[" );
  oja->print("B");
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[\"B\"" );
  oja->print("C");
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[\"B\",\"C\"" );
  oja.terminate();
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[\"B\",\"C\"]" );
  ojo.terminate();
  BOOST_CHECK_EQUAL( ss.str(), "{\"A\":[\"B\",\"C\"]}\n" );
}

