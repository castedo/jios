#ifndef JIOS_EXPRESS_HH
#define JIOS_EXPRESS_HH

namespace jios {


class ojvalue;
class ijvalue;

template<class T>
struct jobject_expresser
{
  static void write(ojvalue & oj, T const& src);
  static void read(ijvalue & ij, T & dest);
};

template<class Derived>
struct jobject_expressible
{
  friend void jios_write(ojvalue & oj, Derived const& src)
  {
    jobject_expresser<Derived>::write(oj, src);
  }

  friend void jios_read(ijvalue & ij, Derived & dest)
  {
    jobject_expresser<Derived>::read(ij, dest);
  }
};


} // namespace jios

#endif

