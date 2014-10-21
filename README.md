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
  be `streamed` before all JSON is available


Example
-------

```cpp
json_out(cout, '\n') << 1 << 2 << "three";

```
outputs
```
1
2
"three"
```


Related Libraries
-----------------

* https://github.com/Loki-Astari/ThorsSerializer
* https://github.com/rsms/jsont

