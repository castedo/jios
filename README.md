jios
====

JSON Input Output Streams

Similar to C++ streams, but the stream elements are structured JSON data rather
than characters.

Features
--------

* streams are backed by source and sink back-ends which do not have to operate
  on textual JSON

* included back-end source and sink interoperate with C++ streams

* interoperability with Google Protocol Buffers as an alternative source and
  sink to JSON text

* source back-end do not have to construct entire json objects in memory;
  instead data can be read directly into C++ objects

* source back-end do not have to parse all JSON at once, but rather parsing can
  be **streamed** before all JSON is available


Parsing Examples
----------------
```cpp
  istringstream ss("1 \"two\" 3");
  int i, j;
  string s;
  json_in(ss) >> i >> s >> j;
```


### Use any type with the istream >> (extraction) operator defined

```cpp
  using namespace boost::posix_time;
  istringstream ss("[\"00:01:03\", \"00:00:12\"]");
  time_duration t1, t2;
  json_in(ss).get().array() >> t1 >> t2;
  cout << boolalpha << bool(t1 == seconds(63)) << ' '
                    << bool(t2 == seconds(12)) << endl;
```
outputs
```
  true true
```


Printing Examples
-----------------

```cpp
  jios::json_out(std::cout, '\n') << 1 << 2 << "three";
```
outputs
```
  1
  2
  "three"
```

In the rest of the examples assume

```cpp
  using namespace std;
  using namespace jios;
  ojstream jout = json_out(cout);
```

### JSON Arrays

```cpp
  ojarray oja = jout.put().array();
  for (int i = 0; i < 3; ++i) {
    oja << i;
  }
  oja << endj;
```
outputs
```
  [0, 1, 2]
```

### JSON Objects

```cpp
  string b = "BEE";
  jout.put().object() << make_pair("one", 1) << tie("two", b) << endj;
```
outputs
```
  {"one":1, "two":"BEE"}
```

### Use any type with the ostream << (insertion) operator defined

```cpp
  using namespace boost::posix_time;
  jout.put().array() << seconds(3) << seconds(2) << seconds(1) << endj;
```
outputs
```
  ["00:00:03", "00:00:02", "00:00:01"]
```

Even use such types as JSON object keys.

```cpp
  using namespace boost::posix_time;
  jout.put().object() << make_pair(seconds(3), "launch")
                      << make_pair(seconds(7), "explode") << endj;
```
outputs
```
  {"00:00:03":"launch", "00:00:07":"explode"}
```


Related Libraries
-----------------

* https://github.com/Loki-Astari/ThorsSerializer
* https://github.com/rsms/jsont

