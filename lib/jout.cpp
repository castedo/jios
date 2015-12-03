#include <jios/jout.hpp>

using namespace std;

namespace jios {


void endj(ojstream & oj)
{
  oj.terminate();
}

void endj(ojobject & oj)
{
  oj.terminate();
}

bool ojstreamoid::at_end() const
{
  return !pimpl_ || pimpl_->do_is_terminator();
}

void ojvalue::write_string_value()
{
  istreambuf_iterator<char> it(buf_);
  istreambuf_iterator<char> end;
  do_print(it, end);
  buf_.clear();
  buf_.str(string());
}

void ojsink::set_key_with_string_value()
{
  istreambuf_iterator<char> it(buf_);
  istreambuf_iterator<char> end;
  do_set_key(it, end);
  buf_.clear();
  buf_.str(string());
}


} // namespace

