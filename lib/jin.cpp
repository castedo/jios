#include <jios/jin.hpp>

#include <limits>

using namespace std;

namespace jios {


ijvalue & ijstream::get()
{
  return *pimpl_;
}

ijvalue const& ijstream::peek()
{
  return *pimpl_;
}

ijpair & ijobject::get()
{
  return *pimpl_;
}

ijpair const& ijobject::peek()
{
  return *pimpl_;
}

std::string ijobject::key()
{
  return pimpl_->do_key();
}

bool ijvalue::ignore()
{
  do_continue();
  return !fail();
}

ijarray ijvalue::array()
{
  ijarray ret = do_begin_array();
  do_continue();
  return ret;
}

ijobject ijvalue::object()
{
  ijobject ret = do_begin_object();
  do_continue();
  return ret;
}

void jios_read(ijvalue & ij, bool & dest)
{
  ij.do_parse(dest);
  ij.do_continue();
}

void jios_read(ijvalue & ij, std::string & dest)
{
  ij.do_parse(dest);
  ij.do_continue();
}

void jios_read(ijvalue & ij, int64_t & dest)
{
  ij.do_parse(dest);
  ij.do_continue();
}

void jios_read(ijvalue & ij, double & dest)
{
  ij.do_parse(dest);
  ij.do_continue();
}

bool ijstreamoid::at_end()
{
  return pimpl_->do_is_terminator() || pimpl_->fail();
}

template<typename T>
void read_int(ijnode & ij, T & dest)
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
void json_copy_if_no_fail(ijnode & src, ojnode & dest)
{
  T tmp;
  if (src.read(tmp)) {
    dest.write(tmp);
  }
}

void jios_read(ijnode & ij, int32_t & out)
{
  read_int(ij, out);
}

void jios_read(ijnode & ij, uint32_t & out)
{
  read_int(ij, out);
}

void jios_read(ijnode & ij, uint64_t & dest)
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

void jios_read(ijnode & ij, float & dest)
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

void jios_read(ijnode & src, ojnode & dest)
{
  switch (src.type()) {
    case json_type::jnull:
      src.ignore();
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

// null_ijnode

class null_ijnode
  : public ijsource
  , public enable_shared_from_this<null_ijnode>
{
  bool fail_;

public:
  null_ijnode() : fail_(false) {}

  bool do_get_failbit() const override { return fail_; }
  void do_set_failbit() override { fail_ = true; }

  json_type do_type() const override { return json_type::jnull; }

  void do_parse(int64_t & dest) override { do_set_failbit(); }
  void do_parse(double & dest) override { do_set_failbit(); }
  void do_parse(bool & dest) override { do_set_failbit(); }
  void do_parse(string & dest) override { do_set_failbit(); }
  void do_continue() override { do_set_failbit(); }

  ijarray do_begin_array() override {
    do_set_failbit();
    return ijarray(shared_from_this());
  }

  ijobject do_begin_object() override {
    do_set_failbit();
    return ijobject(shared_from_this());
  }

  bool do_hint_multiline() const override { return false; }

  bool do_is_terminator() const override { return true; }
  string do_key() const override { return string(); }
};

// ijstreamoid

ijstreamoid::ijstreamoid() : pimpl_(new null_ijnode()) {}


} // namespace

