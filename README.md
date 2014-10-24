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
  using namespace jios;
  jout = json_out(std::cout);
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
  int a = 1;
  string b = "BEE";
  jout.put().object() << tie("one", a) << tie("two", b) << endj;
```
outputs
```
  {"one":1, "two":"BEE"}
```

Related Libraries
-----------------

* https://github.com/Loki-Astari/ThorsSerializer
* https://github.com/rsms/jsont

