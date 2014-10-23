#ifndef JIOS_JIN_HPP
#define JIOS_JIN_HPP

#include <memory>
#include "jout.hpp"

namespace jios {


class ijvalue;
typedef ijvalue ijnode;
class ijpair;
class ijsource;

void jios_read(ijvalue & ij, bool & dest);
void jios_read(ijvalue & ij, std::string & dest);
void jios_read(ijvalue & ij, int64_t & dest);
void jios_read(ijvalue & ij, double & dest);

void jios_read(ijvalue & ij, int32_t & dest);
void jios_read(ijvalue & ij, uint32_t & dest);
void jios_read(ijvalue & ij, uint64_t & dest);
void jios_read(ijvalue & ij, float & dest);
void jios_read(ijvalue & src, ojnode & dest);

//! Stream of JSON-ish values (base for ijarray and ijobject)

class ijstreamoid
  : boost::noncopyable
{
public:
  ijstreamoid();

  ijstreamoid(std::shared_ptr<ijsource> const& pimpl) : pimpl_(pimpl) {}

  ijstreamoid(ijstreamoid && rhs) : pimpl_(rhs.pimpl_) {}

  ijstreamoid & operator = (ijstreamoid && rhs) { pimpl_ = rhs.pimpl_; return *this; }

  bool fail() const;

  void set_failbit();

  bool at_end();

  bool hint_multiline() const;

protected:
  std::shared_ptr<ijsource> pimpl_;
};

class ijstream
  : public ijstreamoid
{
public:
  ijstream() {}

  ijstream(std::shared_ptr<ijsource> const& pimpl) : ijstreamoid(pimpl) {}

  ijvalue & get();

  ijvalue const& peek();

  template<typename T> ijstream & operator >> (T & dest);
};

//! JSON-ish array

class ijarray
  : public ijstream
{
public:
  ijarray() {}

  ijarray(std::shared_ptr<ijsource> const& pimpl) : ijstream(pimpl) {}
};

//! JSON-ish ijobject 

class ijobject
  : public ijstreamoid
{
public:
  ijobject() {}

  ijobject(std::shared_ptr<ijsource> const& pimpl) : ijstreamoid(pimpl) {}

  ijpair & get();

  ijpair const& peek();

  std::string key();

  template<typename T> ijobject & operator >> (T & dest);
};

//! JSON-ish value

enum class json_type {
    jnull,
    jbool,
    jinteger, //! backends must have fidelity of int64_t
    jfloat,   //! backends may have little fidelity for jfloat
    jstring,  //! UTF-8 string
    jarray,
    jobject};

class ijvalue
  : boost::noncopyable
{
public:
  virtual ~ijvalue() {}

  bool fail() const { return do_get_failbit(); }
  void set_failbit() { do_set_failbit(); }

  template<typename T>
  bool read(T & dest) { jios_read(*this, dest); return !fail(); }

  bool ignore();

  ijarray array();
  ijobject object();

  DEPRECATED ijarray begin_array() { return this->array(); }
  DEPRECATED ijobject begin_object() { return this->object(); }

  json_type type() const { return do_type(); }
  bool is_array() const { return json_type::jarray == do_type(); }
  bool is_object() const { return json_type::jobject == do_type(); }

protected:
  ijvalue() : expired_(false) {}

private:
  friend class ijstreamoid;
  friend class ijstream;

  friend void jios_read(ijvalue & ij, bool & dest);
  friend void jios_read(ijvalue & ij, std::string & dest);
  friend void jios_read(ijvalue & ij, int64_t & dest);
  friend void jios_read(ijvalue & ij, double & dest);

  virtual bool do_get_failbit() const = 0;
  virtual void do_set_failbit() = 0;

  virtual json_type do_type() const = 0;

  virtual void do_parse(int64_t & dest) = 0;
  virtual void do_parse(double & dest) = 0;
  virtual void do_parse(bool & dest) = 0;
  virtual void do_parse(std::string & dest) = 0;
  virtual void do_continue() = 0;

  virtual ijarray do_begin_array() = 0;
  virtual ijobject do_begin_object() = 0;
  virtual bool do_hint_multiline() const { return false; }

  bool expired_; // extracted element read, further reads trigger continue
};

class ijpair : public ijvalue
{
public:
  std::string key() const { return do_key(); }

  template<class Map>
  bool read_to_map(Map & m) { return this->read(m[do_key()]); }

private:
  friend class ijobject;

  virtual std::string do_key() const = 0;
};

class ijsource : protected ijpair
{
  friend class ijstreamoid;
  friend class ijstream;
  friend class ijobject;

  virtual bool do_is_terminator() const = 0;
};

////////////////////////////////////////
/// inline method implementations

inline bool ijstreamoid::fail() const
{
  return pimpl_->do_get_failbit();
}

inline void ijstreamoid::set_failbit()
{
  pimpl_->do_set_failbit();
}

template<typename T>
inline ijstream & ijstream::operator >> (T & dest)
{
  pimpl_->read(dest);
  return *this;
}

template<typename T>
inline ijobject & ijobject::operator >> (T & dest)
{
  pimpl_->read(dest);
  return *this;
}

inline bool ijstreamoid::hint_multiline() const
{ 
  return pimpl_->do_hint_multiline();
}


} // namespace

#endif

