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

void jios_write(ojvalue & oj, char src)
{
  oj.write(string(src, 1));
}

void jios_write(ojvalue & oj, char const* src)
{
  oj.write(string(src));
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


} // namespace

