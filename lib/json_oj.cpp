#include <jios/json_oj.hpp>

#include <boost/core/null_deleter.hpp>
#include <boost/optional.hpp>

using namespace std;

namespace jios {


// json escaping

template<class Ch>
void json_escape(std::basic_ostream<Ch> & out, std::basic_string<Ch> const& in)
{
  // Modified version of function from boost PTree code.
  typename std::basic_string<Ch>::const_iterator b = in.begin();
  typename std::basic_string<Ch>::const_iterator e = in.end();
  while (b != e)
  {
      // We escape everything outside ASCII, because this code can't
      // handle high unicode characters.
      if (*b >= 0x20 && *b != '"' && *b != '\\' && *b <= 0xFF) { out << *b; }
      else if (*b == Ch('\b')) out << Ch('\\') << Ch('b');
      else if (*b == Ch('\f')) out << Ch('\\') << Ch('f');
      else if (*b == Ch('\n')) out << Ch('\\') << Ch('n');
      else if (*b == Ch('\r')) out << Ch('\\') << Ch('r');
      else if (*b == Ch('"')) out << Ch('\\') << Ch('"');
      else if (*b == Ch('\\')) out << Ch('\\') << Ch('\\');
      else
      {
          const char *hexdigits = "0123456789ABCDEF";
          typedef typename boost::make_unsigned<Ch>::type UCh;
          unsigned long u = (std::min)(static_cast<unsigned long>(
                                           static_cast<UCh>(*b)),
                                       0xFFFFul);
          int d1 = u / 4096; u -= d1 * 4096;
          int d2 = u / 256; u -= d2 * 256;
          int d3 = u / 16; u -= d3 * 16;
          int d4 = u;
          out << Ch('\\') << Ch('u');
          out << Ch(hexdigits[d1]) << Ch(hexdigits[d2]);
          out << Ch(hexdigits[d3]) << Ch(hexdigits[d4]);
      }
      ++b;
  }
}

// ostream_ojnode

class ostream_ojnode
  : public ojnode
  , public enable_shared_from_this<ostream_ojnode>
  , boost::noncopyable
{
public:
  virtual ~ostream_ojnode() {}

  ostream_ojnode(shared_ptr<ostream> const& os, char delim)
    : os_(os)
    , multimode_(false)
    , o_delim_(delim)
    , state_(CLEARED)
    , precomma_(false)
  {
    init(false);
  }

  ostream_ojnode(shared_ptr<ostream> const& os,
                 shared_ptr<ostream_ojnode> const& parent,
                      bool in_object,
                      bool multimode)
    : os_(os)
    , multimode_(multimode)
    , parent_(parent)
    , state_(CLEARED)
    , precomma_(false)
  {
    init(in_object);
  }

  template<typename T> void do_print_impl(T const& value);

  virtual void do_print_null();
  virtual void do_print(int32_t const& value) { do_print_impl(value); }
  virtual void do_print(uint32_t const& value) { do_print_impl(value); }
  virtual void do_print(int64_t const& value) { do_print_impl(value); }
  virtual void do_print(uint64_t const& value) { do_print_impl(value); }
  virtual void do_print(double const& value) { do_print_impl(value); }
  virtual void do_print(float const& value) { do_print_impl(value); }
  virtual void do_print(bool const& value) { do_print_impl(value); }
  virtual void do_print(char ch) { do_print_impl(std::string(1, ch)); }
  virtual void do_print(char const* p) { do_print_impl(std::string(p)); }
  virtual void do_print(std::string const& value) { do_print_impl(value); }

  virtual ojarray do_begin_array(bool multimode);
  virtual ojobject do_begin_object(bool multimode);

  virtual void do_key(std::string const& k) { prekey_ = k; }

  virtual void do_flush();
  void do_close();
  virtual void do_terminate();

private:
  void init(bool object);
  virtual void post_comma_whitespace() {}
  virtual shared_ptr<ojnode> make_sub_struct(shared_ptr<ostream> const& os,
                                            bool in_object,
                                            bool multimode);
  virtual void pre_close_whitespace() {}

  void do_open();
  void out_prefix();
  void out_suffix();

protected:
  shared_ptr<ostream> const os_;
  bool multimode_;

private:
  shared_ptr<ostream_ojnode> parent_;
  boost::optional<char> const o_delim_;
  enum { CLEARED,   // cleared for printing (again)
         OPENED,    // open structure printing
         TERMINATED // no more printing should be done
  } state_;
  bool precomma_;
  boost::optional<std::string> prekey_;
};

class pretty_ojnode
  : public ostream_ojnode
{
public:
  virtual ~pretty_ojnode() {}

  pretty_ojnode(shared_ptr<ostream> const& os, char delim)
    : ostream_ojnode(os, delim)
    , indent_(0)
  {
  }

private:
  // json array or object
  pretty_ojnode(shared_ptr<ostream> const& os,
       shared_ptr<ostream_ojnode> const& parent,
       bool in_object,
       bool multimode,
       size_t indent)
    : ostream_ojnode(os, parent, in_object, multimode)
    , indent_(indent)
  {
    newline();
  }

  virtual void post_comma_whitespace();
  virtual shared_ptr<ojnode> make_sub_struct(shared_ptr<ostream> const& os,
                                            bool in_object,
                                            bool multimode);
  virtual void pre_close_whitespace();

  void newline();

  size_t indent_;
};

template<typename T>
void json_print(std::ostream & os, T const& value)
{
  os << value;
}

void json_print(std::ostream & os, bool const& value)
{
  os << (value ? "true" : "false");
}

void json_print(std::ostream & os, std::string const& value)
{
  os << '"';
  json_escape(os, value);
  os << '"';
}

void ostream_ojnode::do_print_null()
{
  if (CLEARED == state_) {
    out_prefix();
    *os_ << "null";
    out_suffix();
  } else {
    os_->setstate(std::ios_base::failbit);
  }
}

template<typename T>
void ostream_ojnode::do_print_impl(T const& value)
{
  if (CLEARED == state_) {
    out_prefix();
    json_print(*os_, value);
    out_suffix();
  } else {
    os_->setstate(std::ios_base::failbit);
  }
}

shared_ptr<ojnode>
    ostream_ojnode::make_sub_struct(shared_ptr<ostream> const& os,
                                         bool in_object,
                                         bool multimode)
{
  shared_ptr<ostream_ojnode> sp = shared_from_this();
  return make_shared<ostream_ojnode>(os_, sp, in_object, multimode);
}

void ostream_ojnode::do_open()
{
  if (CLEARED == state_) {
    out_prefix();
    state_ = OPENED;
  } else {
    os_->setstate(std::ios_base::failbit);
  }
}

ojarray ostream_ojnode::do_begin_array(bool multimode)
{
  do_open();
  return this->make_sub_struct(os_, false, multimode);
}

ojobject ostream_ojnode::do_begin_object(bool multimode)
{
  do_open();
  return this->make_sub_struct(os_, true, multimode);
}

void ostream_ojnode::do_flush()
{
  os_->flush();
}

void ostream_ojnode::do_close()
{
  BOOST_ASSERT(OPENED == state_);
  if (OPENED == state_) {
    out_suffix();
    state_ = CLEARED;
  } else {
    os_->setstate(std::ios_base::failbit);
  }
}

void ostream_ojnode::do_terminate()
{
  if (CLEARED == state_) {
    if (!o_delim_) {
      this->pre_close_whitespace();
      bool in_object(prekey_);
      *os_ << (in_object ? '}' : ']');
    }
    state_ = TERMINATED;
  } else {
    os_->setstate(std::ios_base::failbit);
  }
  if (parent_) { parent_->do_close(); }
}

void ostream_ojnode::init(bool in_object)
{
  if (!os_) {
    BOOST_THROW_EXCEPTION(std::runtime_error("invalid null ostream"));
  }
  if (in_object) { prekey_ = ""; }
  if (!o_delim_) {
    *os_ << (in_object ? '{' : '[');
  }
}

void ostream_ojnode::out_prefix()
{
  if (!o_delim_) {
    if (precomma_) {
      *os_ << ',';
      this->post_comma_whitespace();
    } else {
      precomma_ = true;
    }
  }
  if (prekey_) {
    *os_ << '"';
    json_escape(*os_, *prekey_);
    *os_ << '"' << ':';
    prekey_ = "";
  }
}

void ostream_ojnode::out_suffix()
{
  if (o_delim_) {
    if (*o_delim_ != EOF) {
      *os_ << *o_delim_;
      if (*o_delim_ != '\n') {
        *os_ << '\n';
      }
    }
    *os_ << std::flush;
  }
}

// ojarray

ojarray & ojarray::operator = (ojarray && rhs)
{
  pimpl_ = std::move(rhs.pimpl_);
  return *this;
}

void ojarray::terminate()
{
  BOOST_ASSERT(pimpl_);
  if (pimpl_) {
    pimpl_->do_terminate();
  }
}

// ojobject
 
ojobject & ojobject::operator = (ojobject && rhs)
{
  pimpl_ = std::move(rhs.pimpl_);
  return *this;
}

void ojobject::terminate()
{
  BOOST_ASSERT(pimpl_);
  if (pimpl_) {
    pimpl_->do_terminate();
  }
}

// pretty_ojnode

shared_ptr<ojnode>
    pretty_ojnode::make_sub_struct(shared_ptr<ostream> const& os,
                                           bool in_object,
                                           bool multimode)
{
  size_t sub_indent = indent_ + (multimode ? 1 : 0);
  shared_ptr<ostream_ojnode> sp = shared_from_this();
  auto p = new pretty_ojnode(os_, sp, in_object, multimode, sub_indent);
  return shared_ptr<pretty_ojnode>(p);
}

void pretty_ojnode::pre_close_whitespace()
{
  --indent_;
  newline();
}

void pretty_ojnode::post_comma_whitespace()
{
  if (!multimode_) { *os_ << ' '; }
  else { newline(); }
}

void pretty_ojnode::newline()
{
  if (multimode_) {
    *os_ << '\n';
    for (size_t i=0; i < indent_; ++i) { *os_ << '\t'; }
  }
}

// root factory function

ojstream json_out(std::ostream & os, char delim)
{
  shared_ptr<ostream> sp(&os, boost::null_deleter());
  return shared_ptr<ojsink>(new pretty_ojnode(sp, delim));
}

ojstream json_out(shared_ptr<ostream> const& pos, char delim)
{
  return shared_ptr<ojsink>(new pretty_ojnode(pos, delim));
}

ojstream lined_json_out(std::ostream & os)
{
  shared_ptr<ostream> sp(&os, boost::null_deleter());
  return shared_ptr<ojsink>(new ostream_ojnode(sp, '\n'));
}

ojstream lined_json_out(shared_ptr<ostream> const& pos, char delim)
{
  return shared_ptr<ojsink>(new ostream_ojnode(pos, delim));
}


} // namespace

