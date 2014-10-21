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

//! Base for streams of JSON-ish values

class ojstreamoid
  : boost::noncopyable
{
public:
  ojstreamoid();

  ojstreamoid(std::shared_ptr<ojsink> const& p) : pimpl_(p) {}

  ojstreamoid(ojstreamoid && rhs) : pimpl_(std::move(rhs.pimpl_)) {}

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
  : boost::noncopyable
{
public:
  ojarray(ojarray && rhs) : pimpl_(std::move(rhs.pimpl_)) {}

  ojarray & operator = (ojarray && rhs);

  ojnode & operator * () { return *pimpl_; }

  ojnode * operator -> () { return pimpl_.get(); }

  void terminate();

public:
  ojarray(std::shared_ptr<ojsink> const& p) : pimpl_(p) {}

private:
  std::shared_ptr<ojsink> pimpl_;
};

// ojobject

class ojobject
  : boost::noncopyable
{
public:
  ojobject(ojobject && rhs) : pimpl_(std::move(rhs.pimpl_)) {}

  ojobject & operator = (ojobject && rhs);

  ojvalue & put(std::string const& k);

  ojnode & operator [] (std::string const& k);

  template<typename T> ojnode & operator [] (T const& k);

  void terminate();

public:
  ojobject(std::shared_ptr<ojsink> const& p) : pimpl_(p) {}

private:
  std::shared_ptr<ojsink> pimpl_;
};

// ojnode

class ojnode
{
public:
  ~ojnode() {}

  void print_null() { return do_print_null(); }
  void print(int32_t const& value) { return do_print(value); }
  void print(uint32_t const& value) { return do_print(value); }
  void print(int64_t const& value) { return do_print(value); }
  void print(uint64_t const& value) { return do_print(value); }
  void print(long const& value) { return do_print(int64_t(value)); }
  void print(double const& value) { return do_print(value); }
  void print(float const& value) { return do_print(value); }
  void print(bool const& value) { return do_print(value); }
  void print(char ch) { return do_print(ch); }
  void print(char const* p) { return do_print(p); }
  void print(std::string const& value) { return do_print(value); }

  template<typename T> void print(boost::optional<T> const& ov);

  template<typename T>
  void write(T const& src) { return this->print(src); }

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
  virtual void do_print_null() = 0;
  virtual void do_print(int32_t const& value) = 0;
  virtual void do_print(uint32_t const& value) = 0;
  virtual void do_print(int64_t const& value) = 0;
  virtual void do_print(uint64_t const& value) = 0;
  virtual void do_print(double const& value) = 0;
  virtual void do_print(float const& value) = 0;
  virtual void do_print(bool const& value) = 0;
  virtual void do_print(char ch) = 0;
  virtual void do_print(char const* p) = 0;
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

template<typename T>
void ojnode::print(boost::optional<T> const& ov)
{
  if (ov) { this->print(*ov); }
  else { this->print_null(); }
}

////////////////////////////////////////
/// inline method implementations

inline void ojstreamoid::terminate()
{
  if (pimpl_) {
    pimpl_->do_terminate();
  }
}

inline ojvalue & ojstream::put()
{
  return *pimpl_;
}

template<typename T>
inline ojstream & ojstream::operator << (T const& src)
{
  if (pimpl_) {
    pimpl_->print(src);
  }
  return *this;
}


} // namespace

#endif

