JIOS Async Example
==================

In this example, an `async_adder` class will read any available JSON array
elements and keep a running sum. The available JSON can be partial and
by calling `expecting`, client code and detect whether enough JSON is
available to read a complete JSON array member or not.

```cpp

struct async_adder
{
  ijarray ija;
  int sum; 

  void init(istream & is)
  {
    ija = json_in(is).get().array();
    sum = 0;
  }

  void add_without_blocking()
  {
    while (!ija.expecting()) {
      int i;
      if (ija >> i) { sum += i; }
    }
  }

  bool done()
  {
    return ija.at_end();
  }
};


void handle_first_input(istream & source, async_adder & adder)
{
  adder.init(source);
}

void handle_additional_input(async_adder & adder)
{
  if (!adder.done()) {
    adder.add_without_blocking();
    // request more data
    ...
  }
}
```

