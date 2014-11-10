#include <jios/jin.hpp>

#include <limits>

using namespace std;

namespace jios {


ostream & operator << (ostream & os, json_type src)
{
  const char * name = "";
  #define CASE(s) case s: name = #s
  switch (src) {
    CASE(json_type::jnull);
    CASE(json_type::jbool);
    CASE(json_type::jinteger);
    CASE(json_type::jfloat);
    CASE(json_type::jstring);
    CASE(json_type::jarray);
    CASE(json_type::jobject);
  }
  #undef CASE
  return os << name;
}

ijvalue & ijstream::get()
{
  return this->extract();
}

ijvalue const& ijstream::peek()
{
  return this->dereference();
}

void ijstreamoid::null_if_end(ijstreamoid * & p_src)
{
  if (p_src->pimpl_->is_terminator() || p_src->fail()) {
    p_src = nullptr;
  }
}

void ijstreamoid::increment(ijstreamoid * & p_src)
{
  p_src->unexpire();
  p_src->pimpl_->advance();
  null_if_end(p_src);
}

void ijstreamoid::unexpire()
{
  if (expired_) {
    pimpl_->advance();
    expired_ = false;
  }
}

bool ijstreamoid::expecting()
{
  unexpire();
  return pimpl_->expecting();
}

ijpair & ijstreamoid::dereference()
{
  unexpire();
  BOOST_ASSERT(!this->fail());
  BOOST_ASSERT(!pimpl_->is_terminator());
  return pimpl_->dereference();
}

ijpair & ijstreamoid::extract()
{
  ijpair & ret = dereference();
  expired_ = true;
  return ret;
}

ijpair & ijobject::get()
{
  return this->extract();
}

ijpair const& ijobject::peek()
{
  return this->dereference();
}

string ijobject::key()
{
  return this->peek().key();
}

string ijpair::key() const
{
  return do_key();
}

ijarray ijvalue::array()
{
  return do_begin_array();
}

ijobject ijvalue::object()
{
  return do_begin_object();
}

json_type ijvalue::type() const
{
  return do_type();
}

istream & ijvalue::read_string_value()
{
  buf_.clear();
  buf_.str(string());
  do_parse(ostreambuf_iterator<char>(buf_));
  return buf_;
}

istream & ijpair::read_key_value() const
{
  buf_.clear();
  buf_.str(do_key());
  return buf_;
}

bool ijvalue::good_string_value_read()
{
  buf_ >> ws;
  if (buf_.fail() || !buf_.eof()) {
    set_failbit();
  }
  return !fail();
}

void jios_read(ijvalue & ij, bool & dest)
{
  ij.do_parse(dest);
}

void jios_read(ijvalue & ij, std::string & dest)
{
  ij.do_parse(dest);
}

void jios_read(ijvalue & ij, int64_t & dest)
{
  ij.do_parse(dest);
}

void jios_read(ijvalue & ij, double & dest)
{
  ij.do_parse(dest);
}

bool ijstreamoid::at_end()
{
  unexpire();
  return pimpl_->is_terminator() || this->fail();
}

template<typename T>
void read_int(ijvalue & ij, T & dest)
{
  typedef numeric_limits<T> numeric;
  int64_t tmp;
  if (ij.read(tmp)) {
    if (tmp >= numeric::min() && tmp <= numeric::max()) {
      dest = tmp;
    } else {
      ij.set_failbit();
    }
  }
}

template<typename T>
void json_copy_if_no_fail(ijvalue & src, ojvalue & dest)
{
  T tmp;
  if (src.read(tmp)) {
    dest.write(tmp);
  }
}

void jios_read(ijvalue & ij, int32_t & out)
{
  read_int(ij, out);
}

void jios_read(ijvalue & ij, uint32_t & out)
{
  read_int(ij, out);
}

void jios_read(ijvalue & ij, uint64_t & dest)
{
  int64_t tmp;
  if (ij.read(tmp)) {
    if (tmp >= 0) {
      dest = tmp;
    } else {
      ij.set_failbit();
    }
  }
}

void jios_read(ijvalue & ij, float & dest)
{
  typedef numeric_limits<float> numeric;
  double tmp;
  if (ij.read(tmp) && isfinite(tmp)) {
    if (tmp < numeric::min() || tmp > numeric::max()) {
      ij.set_failbit();
    } 
  }
  if (!ij.fail()) {
    dest = tmp;
  }
}

void jios_read(ijvalue & src, ojvalue & dest)
{
  switch (src.type()) {
    case json_type::jnull:
      dest.write_null();
      break;
    case json_type::jbool:
      json_copy_if_no_fail<bool>(src, dest);
      break;
    case json_type::jinteger:
      json_copy_if_no_fail<int64_t>(src, dest);
      break;
    case json_type::jfloat:
      json_copy_if_no_fail<double>(src, dest);
      break;
    case json_type::jstring:
      json_copy_if_no_fail<string>(src, dest);
      break;
    case json_type::jarray:
      {
        ijarray ija = src.array();
        ojarray oja = dest.array(ija.hint_multiline());
        while (!ija.at_end()) { ija >> *oja; }
        oja.terminate();
      }
      break;
    case json_type::jobject:
      {
        ijobject ijo = src.object();
        ojobject ojo = dest.object(ijo.hint_multiline());
        while (!ijo.at_end()) {
          ijo.get().read_to_map(ojo);
        }
        ojo.terminate();
      }
      break;
  }
}

// null_ijsource

class null_ijsource
  : public ijsource
  , public enable_shared_from_this<null_ijsource>
  , private ijpair
  , private ijstate
{
public:
  null_ijsource() {}

private:
  void debug() const {
    BOOST_ASSERT_MSG(false, "null_ijsource accessed");
  } 

  ijstate & do_state() override { return *this; }
  ijstate const& do_state() const override { return *this; }

  ijpair & do_ref() override { return *this; }

  bool do_get_failbit() const override { return true; }
  void do_set_failbit() override { debug(); }
  json_type do_type() const override { debug(); return json_type::jnull; }
  void do_parse(int64_t & dest) override { debug(); }
  void do_parse(double & dest) override { debug(); }
  void do_parse(bool & dest) override { debug(); }
  void do_parse(string & dest) override { debug(); }
  void do_parse(buffer_iterator) override { debug(); }
  ijarray do_begin_array() override {
    debug();
    return ijarray(shared_from_this());
  }
  ijobject do_begin_object() override {
    debug();
    return ijobject(shared_from_this());
  }
  bool do_hint_multiline() const override { debug(); return false; }
  void do_advance() override { debug(); }
  bool do_expecting() override { debug(); return false; }
  bool do_is_terminator() override { return true; }
  string do_key() const override { debug(); return string(); }
};

// ijstreamoid

ijstreamoid::ijstreamoid()
  : pimpl_(new null_ijsource())
  , expired_(false)
{}

ijstreamoid::ijstreamoid(std::shared_ptr<ijsource> const& pimpl)
  : pimpl_(pimpl ? pimpl : make_shared<null_ijsource>())
  , expired_(false)
{}

ijstreamoid::ijstreamoid(ijstreamoid && rhs)
  : pimpl_(rhs.pimpl_)
  , expired_(false)
{}

ijstreamoid & ijstreamoid::operator = (ijstreamoid && rhs)
{
  pimpl_ = rhs.pimpl_;
  expired_ = rhs.expired_;
  return *this;
}

// ijsource

ijpair & ijsource::dereference()
{
  BOOST_ASSERT(!this->fail());
  BOOST_ASSERT(!this->is_terminator());
  return do_ref();
}

bool ijsource::is_terminator()
{
  bool ret = do_is_terminator();
  BOOST_ASSERT( !this->fail() || ret );
  return ret || this->fail();
}

void ijsource::advance()
{
  bool end = this->is_terminator();
  BOOST_ASSERT(!end);
  if (!end) {
    do_advance();
  }
}

bool ijsource::expecting()
{
  bool ret = do_expecting();
  BOOST_ASSERT( !this->fail() || !ret );
  return ret && !this->fail();
}


} // namespace

