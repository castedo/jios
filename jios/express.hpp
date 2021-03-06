#ifndef JIOS_EXPRESS_HPP
#define JIOS_EXPRESS_HPP

#include "express.hh"
#include "jin.hpp"
#include "jout.hpp"

#include <utility>

namespace jios {


////////////////////////////////////////////////////////////////////
// jios_express detection

namespace detail {

  template<class Base, class Derived>
  using EnabledIfIsBaseOf = typename
     std::enable_if<std::is_base_of<Base, Derived>::value>::type;

  struct fake_expression {};

  template<class T>
  using JiosExpressReturnType =
     decltype(T::jios_express(std::declval<fake_expression&>()));

  template<typename T, typename ReturnType = JiosExpressReturnType<T>>
  struct jobject_jios_express
  {
      typedef match_tag tag;
      static_assert( std::is_void<ReturnType>::value,
                     "jios_express return type should be void" );
  };

}

template<typename T, typename Omitted = detail::match_tag>
struct is_jobject_expressible
  : std::false_type
{};

template<typename T>
struct is_jobject_expressible<T, typename detail::jobject_jios_express<T>::tag>
  : std::true_type
{};

//! JSON object expressing classes

template<class T, typename Omitted = std::true_type>
struct jobject_clearer
{
public:
  static void clear(T & dest) { dest = T(); }
};

template<class T>
struct jobject_clearer<T, typename is_jobject_expressible<T>::type>
{
  static
  void clear(T & dest)
  {
    jobject_clearer clearer(dest);
    T::jios_express(clearer);
  }

  template<class MemberT, class BaseT,
           class = detail::EnabledIfIsBaseOf<BaseT, T>>
  jobject_clearer & member(std::string const& key, MemberT BaseT::*mptr)
  {
    BaseT & base = dest_;
    MemberT & data = base.*mptr;
    jobject_clearer<MemberT>::clear(data);
    return *this;
  }

private:
  jobject_clearer(T & dest) : dest_(dest) {}

  T & dest_;
};

template<class T, class Expresser = T>
struct jobject_writer
{
  static
  void write(ojvalue & oj, T const& src)
  {
    jobject_writer writer(oj.object(true), src);
    Expresser::jios_express(writer);
    writer.ojo_ << endj;
  }

  template<class MemberT, class BaseT,
           class = detail::EnabledIfIsBaseOf<BaseT, T>>
  jobject_writer & member(std::string const& key, MemberT BaseT::*mptr)
  {
    BaseT const& base = src_;
    MemberT const& data = base.*mptr;
    ojo_ << std::tie(key, data);
    return *this;
  }

private:
  jobject_writer(ojobject && dest, T const& src)
    : ojo_(std::move(dest))
    , src_(src)
  {}

  ojobject ojo_;
  T const& src_;
};

template<class T, class Expresser = T>
struct jobject_reader
{
  static
  void read(ijvalue & ij, T & dest)
  {
    jobject_clearer<Expresser>::clear(dest);
    merge(ij, dest);
  }

  static
  void merge(ijvalue & ij, T & dest)
  {
    jobject_reader reader(ij.object(), dest);
    while (!reader.ijo_.fail() && !reader.ijo_.at_end()) {
      Expresser::jios_express(reader);
      if (!reader.key_found_) {
        reader.ijo_.set_failbit();
      } else {
        reader.key_found_ = false;
      }
    }
  }

  template<class MemberT, class BaseT,
           class = detail::EnabledIfIsBaseOf<BaseT, T>>
  jobject_reader & member(std::string const& key, MemberT BaseT::*mptr)
  {
    BaseT & base = dest_;
    MemberT & data = base.*mptr;
    if (!key_found_ && ijo_.key() == key) {
      key_found_ = true;
      ijo_.get().read(data);
    }
    return *this;
  } 
private:
  jobject_reader(ijobject && src, T & dest)
    : ijo_(std::move(src))
    , dest_(dest)
    , key_found_(false)
  {}

  ijobject ijo_;
  T & dest_;
  bool key_found_;
};

// jobject_expresser implementation

template<class T>
void jobject_expresser<T>::write(ojvalue & oj, T const& src)
{
  jobject_writer<T>::write(oj, src);
}

template<class T>
void jobject_expresser<T>::read(ijvalue & ij, T & dest)
{
  jobject_reader<T>::read(ij, dest);
}

//! JSON tuple (array) expressing classes

template<class T, class Expresser = T>
class jtuple_writer
{
public:
  static
  void write(ojvalue & oj, T const& src)
  {
    jtuple_writer writer(oj.array(), src);
    Expresser::jios_express(writer);
    writer.oja_ << endj;
  }

  template<typename MemberT, class Base>
  jtuple_writer & member(MemberT Base::*mptr)
  {
    MemberT const& data = src_.*mptr;
    oja_ << data;
    return *this;
  }

private:
  jtuple_writer(ojarray && dest, T const& src)
    : oja_(std::move(dest))
    , src_(src)
  {}

  ojarray oja_;
  T const& src_;
};

template<class T, class Expresser = T>
class jtuple_reader
{
public:
  static
  void read(ijvalue & ij, T & dest)
  {
    dest = T();
    jtuple_reader reader(ij.array(), dest);
    while (!reader.fail() && !reader.at_end()) {
      Expresser::jios_express(reader);
    }
  }

  template<typename MemberT, class Base>
  jtuple_reader & member(MemberT Base::*mptr)
  {
    MemberT & data = dest_.*mptr;
    ijo_ >> data;
    return *this;
  }

private:
  jtuple_reader(ijarray && src, T & dest)
    : ijo_(std::move(src))
    , dest_(dest)
  {}

  bool at_end() { return ijo_.at_end(); }

  bool fail() const { return ijo_.fail(); }

  ijarray ijo_;
  T & dest_;
};


} // namespace jios

#endif
