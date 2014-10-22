#ifndef JIOS_JOUT_HPP
#define JIOS_JOUT_HPP

#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

namespace jios {


class ojnode;
typedef ojnode ojvalue;
typedef ojnode ojsink;

void jios_write(ojvalue & oj, bool src);
void jios_write(ojvalue & oj, std::string const& src);
void jios_write(ojvalue & oj, int64_t src);
void jios_write(ojvalue & oj, double src);

void jios_write(ojvalue & oj, char src);
void jios_write(ojvalue & oj, char const* src);
void jios_write(ojvalue & oj, int32_t src);
void jios_write(ojvalue & oj, uint32_t src);
void jios_write(ojvalue & oj, long src);
void jios_write(ojvalue & oj, float src);

template<typename T>
void jios_write(boost::optional<T> const& ov);

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
};

// ojarray

class ojarray
  : public ojstream
{
public:
  ojarray(std::shared_ptr<ojsink> const& p) : ojstream(p) {}

  ojnode & operator * () { return *pimpl_; }

  ojnode * operator -> () { return pimpl_.get(); }
};

// ojobject

class ojobject
  : public ojstreamoid
{
public:
  ojobject(std::shared_ptr<ojsink> const& p) : ojstreamoid(p) {}

  ojvalue & put(std::string const& k);

  ojnode & operator [] (std::string const& k);

  template<typename T> ojnode & operator [] (T const& k);
};

// ojnode

class ojnode
{
public:
  ~ojnode() {}

  void write_null() { do_print_null(); }

  template<typename T>
  void write(T const& src) { jios_write(*this, src); }

  void print(std::string const& value) { this->write(value); }

  ojarray array(bool multimode = false)
  {
     return do_begin_array(multimode);
  }

  ojobject object(bool multimode = false)
  {
     return do_begin_object(multimode);
  }

  ojarray begin_array(bool multimode = false)
  {
     return do_begin_array(multimode);
  }

  ojobject begin_object(bool multimode = false)
  {
     return do_begin_object(multimode);
  }

  void flush() { do_flush(); }

protected:
  friend void jios_write(ojvalue & oj, bool src);
  friend void jios_write(ojvalue & oj, std::string const& src);
  friend void jios_write(ojvalue & oj, int64_t src);
  friend void jios_write(ojvalue & oj, double src);

  virtual void do_print_null() = 0;
  virtual void do_print(int64_t value) = 0;
  virtual void do_print(double value) = 0;
  virtual void do_print(bool value) = 0;
  virtual void do_print(std::string const& value) = 0;

  virtual ojarray do_begin_array(bool multimode) = 0;
  virtual ojobject do_begin_object(bool multimode) = 0;

  friend class ojstreamoid;
  friend class ojstream;
  friend class ojarray;
  friend class ojobject;

  virtual void do_flush() = 0;
  virtual void do_terminate() = 0;
  virtual void do_key(std::string const& k) = 0;
};

////////////////////////////////////////
/// inline method implementations

inline
ojvalue & ojobject::put(std::string const& k)
{
  pimpl_->do_key(k);
  return *pimpl_;
}

inline ojnode & ojobject::operator [] (std::string const& k)
{
  pimpl_->do_key(k);
  return *pimpl_;
}

template<typename T>
ojnode & ojobject::operator [] (T const& k)
{
  pimpl_->do_key(boost::lexical_cast<std::string>(k));
  return *pimpl_;
}

inline
void jios_write(ojvalue & oj, bool src)
{
  oj.do_print(src);
}

inline
void jios_write(ojvalue & oj, std::string const& src)
{
  oj.do_print(src);
}

inline
void jios_write(ojvalue & oj, int64_t src)
{
  oj.do_print(src);
}

inline
void jios_write(ojvalue & oj, double src)
{
  oj.do_print(src);
}

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

template<typename T> inline
ojstream & ojstream::operator << (T const& src)
{
  if (pimpl_) {
    pimpl_->write(src);
  }
  return *this;
}


} // namespace

#endif

