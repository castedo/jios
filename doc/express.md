JIOS Expression Example
=======================


```cpp
  #include <jios/express.hpp>

  using namespace std;

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

  int main()
  {
    stringstream ss;
    ss << R"( { "name": "Joe", "age": 32 } )";
    person joe;
    jios::json_in(ss) >> joe;
    joe.name += " Smoe";
    joe.age += 8;
    jios::json_out(cout) << joe;
  }
```
outputs
```
{
    "name": "Joe Smoe",
    "age": 40
}
```

