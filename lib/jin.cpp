#include <jios/jin.hpp>

#include <limits>

using namespace std;

namespace jios {


void ijstreamoid::advance()
{
  pimpl_->do_advance();
  pimpl_->do_ref().expired_ = false;
}

ijvalue & ijstream::get()
{
  BOOST_VERIFY(!this->at_end());
  return pimpl_->do_ref();
}

ijvalue const& ijstream::peek()
{
  BOOST_VERIFY(!this->at_end());
  return pimpl_->do_peek();
}

void ijstreamoid::increment(ijsource * & p_src)
{
  p_src->do_advance();
  p_src->do_ref().expired_ = false;
  if (p_src->do_is_terminator() || p_src->fail()) {
    p_src = nullptr;
  }
}

ijpair & ijobject::get()
{
  BOOST_VERIFY(!this->at_end());
  return pimpl_->do_ref();
}

ijpair const& ijobject::peek()
{
  BOOST_VERIFY(!this->at_end());
  return pimpl_->do_peek();
}

string ijobject::key()
{
  BOOST_VERIFY(!this->at_end());
  return pimpl_->do_peek().key();
}

string ijpair::key() const
{
  return do_key();
}

void ijvalue::extraction_expiration_boundary()
{
  BOOST_ASSERT(!expired_);
  expired_ = true;
}

ijarray ijvalue::array()
{
  extraction_expiration_boundary();
  return do_begin_array();
}

ijobject ijvalue::object()
{
  extraction_expiration_boundary();
  return do_begin_object();
}

json_type ijvalue::type() const
{
  return do_type();
}

istream & ijvalue::read_string_value()
{
  extraction_expiration_boundary();
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
  ij.extraction_expiration_boundary();
  ij.do_parse(dest);
}

void jios_read(ijvalue & ij, std::string & dest)
{
  ij.extraction_expiration_boundary();
  ij.do_parse(dest);
}

void jios_read(ijvalue & ij, int64_t & dest)
{
  ij.extraction_expiration_boundary();
  ij.do_parse(dest);
}

void jios_read(ijvalue & ij, double & dest)
{
  ij.extraction_expiration_boundary();
  ij.do_parse(dest);
}

bool ijstreamoid::at_end()
{
  if (pimpl_->do_peek().expired_) { advance(); }
  return pimpl_->do_is_terminator() || pimpl_->fail();
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
{
public:
  null_ijsource() {}

private:
  void debug() const {
    BOOST_ASSERT_MSG(false, "null_ijsource accessed");
  } 

  ijpair & do_ref() override { return *this; }
  ijpair const& do_peek() override { return *this; }

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
  bool do_ready() override { debug(); return false; }
  bool do_is_terminator() override { return true; }
  string do_key() const override { debug(); return string(); }
};

// ijstreamoid

ijstreamoid::ijstreamoid() : pimpl_(new null_ijsource()) {}


} // namespace

