#include <boost/test/unit_test.hpp>

#include <jios/json_in.hpp>
#include <jios/json_out.hpp>
#include <jios/express.hpp>

using namespace std;
using namespace jios;


struct person
  : private jios::jobject_expressible<person>
{
  string name;
  int age;

  template<class Expression>
  static
  void jios_express(Expression & exp)
  {
    exp.member("name", &person::name)
       .member("age", &person::age);
  }
};

BOOST_AUTO_TEST_CASE( express_in_test )
{
  stringstream ss;
  ss << R"( { "name":"Joe", "age":32 } )";
  person joe;
  json_in(ss) >> joe;
  BOOST_CHECK_EQUAL( joe.name, "Joe" );
  BOOST_CHECK_EQUAL( joe.age, 32 );
}

BOOST_AUTO_TEST_CASE( express_out_test )
{
  person joe;
  joe.name = "Joe";
  joe.age = 32;

  ostringstream os;
  json_out(os) << joe;
  BOOST_CHECK_EQUAL( os.str(), "{\n\t\"name\":\"Joe\",\n\t\"age\":32\n}" );
}

