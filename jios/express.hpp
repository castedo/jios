#ifndef JIOS_EXPRESS_HPP
#define JIOS_EXPRESS_HPP

#include "jin.hpp"
#include "jout.hpp"

namespace jios {


//! JSON object expressing classes

template<class T, class Expresser = T>
class jobject_writer
{
public:
  static
  void write(ojvalue & oj, T const& src)
  {
    jobject_writer writer(oj.object(true), src);
    Expresser::jios_express(writer);
    writer.ojo_ << endj;
  }

  template<typename MemberT>
  jobject_writer & member(std::string const& key, MemberT T::*mptr)
  {
    MemberT const& data = src_.*mptr;
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
class jobject_reader
{
public:
  static
  void read(ijvalue & ij, T & dest)
  {
    dest = T();
    jobject_reader<T> reader(ij.object(), dest);
    while (!reader.ijo_.at_end() && !reader.ijo_.fail()) {
      Expresser::jios_express(reader);
      if (!reader.key_found_) {
        reader.ijo_.set_failbit();
      } else {
        reader.key_found_ = false;
      }
    }
  }

  template<typename MemberT>
  jobject_reader & member(std::string const& key, MemberT T::*mptr)
  {
    MemberT & data = dest_.*mptr;
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


template<class Derived>
struct jobject_expressible
{
  friend void jios_write(ojvalue & oj, Derived const& src)
  {
    jobject_writer<Derived>::write(oj, src);
  }

  friend void jios_read(ijvalue & ij, Derived & dest)
  {
    jobject_reader<Derived>::read(ij, dest);
  }
};

//! JSON tuble (array) expressing classes

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
    while (!reader.at_end() && !reader.fail()) {
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
