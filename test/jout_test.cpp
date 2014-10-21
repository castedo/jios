#include <boost/test/unit_test.hpp>

#include <jios/json_oj.hpp>

using namespace std;
using namespace jios;

// streamed

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
  ojarray oja = ojo["A"].begin_array(true);
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
