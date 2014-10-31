#ifndef JIOS_JIN_HPP
#define JIOS_JIN_HPP

#include <memory>
#include <sstream>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>
#include <array>
#include <tuple>
#include <boost/noncopyable.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include "jout.hpp"

namespace jios {


class ijvalue;
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
void jios_read(ijvalue & src, ojvalue & dest);

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

  void advance();

  bool hint_multiline() const;

protected:
  template<class Reference>
  class basic_iterator
    : public boost::iterator_adaptor<basic_iterator<Reference>,
                                     ijsource *,
                                     Reference,
                                     boost::single_pass_traversal_tag,
                                     Reference>
  {
  public:
    basic_iterator() : basic_iterator::iterator_adaptor_(nullptr) {}
    basic_iterator(ijsource * p) : basic_iterator::iterator_adaptor_(p) {}
  private:
    friend class boost::iterator_core_access;
    void increment() { ijstreamoid::increment(this->base_reference()); }
    Reference dereference() const { return *(this->base_reference()); }
  };

  static void increment(ijsource * & p_src);

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

  typedef basic_iterator<ijvalue &> iterator;

  iterator begin() { return iterator(pimpl_.get()); }
  iterator end() { return iterator(); }
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

  template<typename KeyT, typename ValT>
  ijobject & operator >> (std::tuple<KeyT &, ValT &> const& dest);

  typedef basic_iterator<ijpair &> iterator;

  iterator begin() { return iterator(pimpl_.get()); }
  iterator end() { return iterator(); }
};

////////////////////////////////////////////////////////////////////
// jios_read detection

namespace detail {
    template<typename T, typename ReturnType = decltype(
        jios_read(std::declval<ijvalue&>(), std::declval<T&>())
    )>
    struct find_jios_read
    {
        typedef found_tag tag;
        static_assert( std::is_void<ReturnType>::value,
                       "jios_read return type should be void" );
    };
}

template<typename T, typename Omitted = detail::found_tag>
struct jios_read_exists
  : std::false_type
{};

template<typename T>
struct jios_read_exists<T, typename detail::find_jios_read<T>::tag>
  : std::true_type
{};

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
  typename std::enable_if<jios_read_exists<T>::value, bool>::type
    read(T & dest)
  {
    jios_read(*this, dest);
    return !fail();
  }

  template<typename T>
  typename std::enable_if<!jios_read_exists<T>::value, bool>::type
    read(T & dest)
  {
    read_string_value() >> dest;
    return good_string_value_read();
  }

  bool ignore();

  ijarray array();
  ijobject object();

  json_type type() const;
  bool is_array() const { return json_type::jarray == this->type(); }
  bool is_object() const { return json_type::jobject == this->type(); }

  bool ready() { return do_ready(); }

protected:
  ijvalue() : expired_(false) {}

  typedef std::ostreambuf_iterator<char> buffer_iterator;

private:
  friend class ijstreamoid;
  friend class ijpair;

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
  virtual void do_parse(buffer_iterator dest) = 0;

  virtual ijarray do_begin_array() = 0;
  virtual ijobject do_begin_object() = 0;
  virtual bool do_hint_multiline() const { return false; }

  virtual void do_advance() = 0;
  virtual bool do_ready() = 0;

  void extraction_expiration_boundary();
  bool expired_; // extracted element read, another read triggers advance

  std::istream & read_string_value();
  bool good_string_value_read();
  mutable std::stringstream buf_;
};

class ijpair : public ijvalue
{
public:
  std::string key() const;

  bool parse_key(std::string & dest) const;

  template<class T>
  bool parse_key(T & dest) const;

  template<class Map>
  bool read_to_map(Map & m) { return this->read(m[this->key()]); }

private:
  friend class ijobject;

  std::istream & read_key_value() const;

  virtual std::string do_key() const = 0;
};

class ijsource : protected ijpair
{
  friend class ijstreamoid;
  friend class ijstream;
  friend class ijobject;

  virtual bool do_is_terminator() = 0;
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

template<typename KeyT, typename ValT>
ijobject & ijobject::operator >> (std::tuple<KeyT &, ValT &> const& dest)
{
  ijpair & kval = this->get();
  if (kval.parse_key(std::get<0>(dest))) {
    kval.read(std::get<1>(dest));
  } else {
    set_failbit();
  }
  return *this;
}

inline bool ijstreamoid::hint_multiline() const
{ 
  return pimpl_->do_hint_multiline();
}

inline bool ijpair::parse_key(std::string & dest) const
{
  dest = this->key();
  return true;
}

template<class T>
bool ijpair::parse_key(T & dest) const
{
  read_key_value() >> dest;
  return !buf_.fail();
}

/////////////////////////////////////////////////////////////////
/// more jios_read definitions for fundamental types

template<class Container>
void jios_read(ijvalue & ij, std::back_insert_iterator<Container> it)
{
  ijarray ija = ij.array();
  while (!ija.at_end()) {
    typename Container::value_type v;
    ija >> v;
    *it = v;
    ++it;
  }
}

template<class T>
void jios_read(ijvalue & ij, std::vector<T> & container)
{
  container.clear();
  jios::jios_read(ij, std::back_inserter(container));
}

template<class T>
void jios_read(ijvalue & ij, std::list<T> & container)
{
  container.clear();
  jios::jios_read(ij, std::back_inserter(container));
}

template<class T>
void jios_read(ijvalue & ij, std::deque<T> & container)
{
  container.clear();
  jios::jios_read(ij, std::back_inserter(container));
}

template<class T, std::size_t N>
void jios_read(ijvalue & ij, std::array<T, N> & container)
{
  container.clear();
  jios::jios_read(ij, std::back_inserter(container));
}

template<class T>
void jios_read(ijvalue & ij, std::forward_list<T> & container)
{
  container.clear();
  jios::jios_read(ij, std::back_inserter(container));
}


} // namespace

#endif

