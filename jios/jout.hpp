#ifndef JIOS_JOUT_HPP
#define JIOS_JOUT_HPP

#include <memory>
#include <tuple>
#include <string>
#include <sstream>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#define DEPRECATED __attribute__((deprecated))

namespace jios {


class ojvalue;
class ojsink;

//! Base for streams of JSON-ish values

class ojstreamoid
  : boost::noncopyable
{
public:
  ojstreamoid();

  ojstreamoid(std::shared_ptr<ojsink> const& p) : pimpl_(p) {}

  ojstreamoid(ojstreamoid && rhs) : pimpl_(std::move(rhs.pimpl_)) {}

  ojstreamoid & operator = (ojstreamoid && rhs);

  void terminate();

protected:
  std::shared_ptr<ojsink> pimpl_;
};

class ojstream
  : public ojstreamoid
{
public:
  ojstream();

  ojstream(std::shared_ptr<ojsink> const& p) : ojstreamoid(p) {}

  ojvalue & put();

  template<typename T> ojstream & operator << (T const& src);

  ojstream & operator << (void (*func)(ojstream &));
};

// ojarray

class ojarray
  : public ojstream
{
public:
  ojarray(std::shared_ptr<ojsink> const& p) : ojstream(p) {}

  ojvalue & operator * ();

  ojvalue * operator -> ();
};

// ojobject

class ojobject
  : public ojstreamoid
{
public:
  ojobject(std::shared_ptr<ojsink> const& p) : ojstreamoid(p) {}

  template<typename T>
  ojvalue & put(T const& key);

  template<typename T>
  ojvalue & operator [] (T const& k) { return this->put(k); }

  template<typename KeyT, typename ValT>
  ojobject & operator << (std::tuple<KeyT, ValT> const& src);

  template<typename KeyT, typename ValT>
  ojobject & operator << (std::pair<KeyT, ValT> const& src);

  ojobject & operator << (void (*func)(ojobject &));
};

////////////////////////////////////////////////////////////////////
// jios_write detection

//! jios_write defines how a type is output as any JSON value.

void jios_write(ojvalue & oj, std::nullptr_t);

namespace detail {

    struct found_tag {};

    template<typename T, typename ReturnType = decltype(
        jios_write(std::declval<ojvalue&>(), std::declval<T>())
    )>
    struct find_jios_write
    {
        typedef found_tag tag;
        static_assert( std::is_void<ReturnType>::value,
                       "jios_write return type should be void" );
    };

}

template<typename T, typename Omitted = detail::found_tag>
struct jios_write_exists
  : std::false_type
{};

template<typename T>
struct jios_write_exists<T, typename detail::find_jios_write<T>::tag>
  : std::true_type
{};

// ojvalue

class ojvalue
{
public:
  virtual ~ojvalue() {}

  void write_null() { do_print_null(); }
  void write_bool(bool b) { do_print(b); }
  void write_int(int64_t i) { do_print(i); }
  void write_double(double d) { do_print(d); }
  template<typename T> void write_string(T const& src);

  template<typename T>
  typename std::enable_if<jios_write_exists<T>::value, void>::type
    write(T const& src)
  {
    jios_write(*this, src);
  }

  //! if no jios_write exists, write type T as JSON string
  template<typename T>
  typename std::enable_if<!jios_write_exists<T>::value, void>::type
    write(T const& src)
  {
    this->write_string(src);
  }

  DEPRECATED void print(std::string const& value) { this->write(value); }

  ojarray array(bool multimode = false)
  {
     return do_begin_array(multimode);
  }

  ojobject object(bool multimode = false)
  {
     return do_begin_object(multimode);
  }

  void flush() { do_flush(); }

protected:
  typedef std::istreambuf_iterator<char> string_iterator;

private:
  virtual void do_print_null() = 0;
  virtual void do_print(int64_t value) = 0;
  virtual void do_print(double value) = 0;
  virtual void do_print(bool value) = 0;
  virtual void do_print(string_iterator, string_iterator) = 0;

  virtual ojarray do_begin_array(bool multimode) = 0;
  virtual ojobject do_begin_object(bool multimode) = 0;

  friend class ojstreamoid;
  friend class ojstream;
  friend class ojarray;
  friend class ojobject;
  friend class ojsink;

  virtual void do_flush() = 0;

  void write_string_value();
  std::stringstream buf_;
};

class ojsink
  : protected ojvalue
{
  friend class ojstreamoid;
  friend class ojstream;
  friend class ojarray;
  friend class ojobject;

  void set_key_with_string_value();

  virtual void do_terminate() = 0;
  virtual void do_set_key(string_iterator, string_iterator) = 0;
};

void endj(ojstream & oj);
void endj(ojobject & oj);

////////////////////////////////////////
/// inline method implementations

inline ojvalue & ojarray::operator * () { return *pimpl_; }

inline ojvalue * ojarray::operator -> () { return pimpl_.get(); }

template<typename T>
ojvalue & ojobject::put(T const& key)
{
  pimpl_->buf_ << key;
  pimpl_->set_key_with_string_value();
  return *pimpl_;
}

template<typename T>
void ojvalue::write_string(T const& src)
{
  buf_ << src;
  write_string_value();
}

/////////////////////////////////////////////////////////////////
/// jios_write definitions for fundamental types

//! Other types can have special jios_write handling if they should not
//! be serialized to JSON strings by default

inline void jios_write(ojvalue & oj, std::nullptr_t)
{
  oj.write_null();
}

template<typename T, typename Omitted = std::true_type>
struct write_arithmetic
{
  //! This is the fallback write for arithmetic types
  static void write(ojvalue & oj, T const& src) { oj.write_string(src); }
};

template<>
struct write_arithmetic<bool, std::true_type>
{
  static void write(ojvalue & oj, bool src) { oj.write_bool(src); }
};

template<>
struct write_arithmetic<char, std::true_type>
{
  static void write(ojvalue & oj, char src) { oj.write_string(src); }
};

template<typename T>
struct write_arithmetic<T, typename std::is_floating_point<T>::type>
{
  static void write(ojvalue & oj, T src) { oj.write_double(src); }
};

template<typename T>
struct write_arithmetic<T, typename std::is_integral<T>::type>
{
  static void write(ojvalue & oj, T src) { oj.write_int(src); }
};

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type
  jios_write(ojvalue & oj, T src)
{
  write_arithmetic<T>::write(oj, src);
};

template<typename T>
void jios_write(ojvalue & oj, boost::optional<T> const& ov)
{
  if (ov) {
    T const& v = *ov;
    oj.write(v);
  } else {
    oj.write_null();
  }
}

inline
ojstreamoid & ojstreamoid::operator = (ojstreamoid && rhs)
{
  pimpl_ = std::move(rhs.pimpl_);
  return *this;
}

inline
void ojstreamoid::terminate()
{
  BOOST_ASSERT(pimpl_);
  if (pimpl_) {
    pimpl_->do_terminate();
  }
}

inline
ojvalue & ojstream::put()
{
  return *pimpl_;
}

template<typename T>
ojstream & ojstream::operator << (T const& src)
{
  if (pimpl_) {
    pimpl_->write(src);
  }
  return *this;
}

inline
ojstream & ojstream::operator << (void (*func)(ojstream &))
{
  (*func)(*this);
  return *this;
}

template<typename KeyT, typename ValT>
ojobject & ojobject::operator << (std::tuple<KeyT, ValT> const& src)
{
  if (pimpl_) {
    ojvalue & oj = this->put(std::get<0>(src));
    oj.write(std::get<1>(src));
  }
  return *this;
}

template<typename KeyT, typename ValT>
ojobject & ojobject::operator << (std::pair<KeyT, ValT> const& src)
{
  if (pimpl_) {
    ojvalue & oj = this->put(std::get<0>(src));
    oj.write(std::get<1>(src));
  }
  return *this;
}

inline
ojobject & ojobject::operator << (void (*func)(ojobject &))
{
  (*func)(*this);
  return *this;
}


} // namespace

#endif

