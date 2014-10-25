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

void jios_write(ojvalue & oj, int32_t src)
{
  oj.write(int64_t(src));
}

void jios_write(ojvalue & oj, uint32_t src)
{
  oj.write(int64_t(src));
}

void jios_write(ojvalue & oj, long src)
{
  oj.write(int64_t(src));
}

void jios_write(ojvalue & oj, float src)
{
  oj.write(double(src));
}

void ojvalue::write_string()
{
  istreambuf_iterator<char> it(buf_.rdbuf());
  istreambuf_iterator<char> end;
  do_print(it, end);
  buf_.clear();
  buf_.str(string());
}

ostream & ojvalue::string_value()
{
  return buf_;
}


} // namespace

