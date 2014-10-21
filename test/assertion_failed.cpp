
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>

class assert_failure : public virtual std::exception, public virtual boost::exception
{
  std::string msg_;
public:
  explicit assert_failure(char const* expr, char const* func,
                          char const* file, long line)
  {
    std::ostringstream ss;
    ss << "Assert failed: (" << expr << ") "
       << func << ' ' << file << ':' << line;
    msg_ = ss.str();
  }
  explicit assert_failure(char const* expr, char const* msg, char const* func,
                          char const* file, long line)
  {
    std::ostringstream ss;
    ss << "Assert failed: (" << expr << ") '" << msg << "' "
       << func << ' ' << file << ':' << line;
    msg_ = ss.str();
  }
  virtual ~assert_failure() throw() {}
  virtual char const * what() const throw() { return msg_.c_str(); }
};

namespace boost {

void assertion_failed(char const* expr, char const* func,
                      char const* file, long line)
{
  throw_exception( assert_failure(expr, func, file, line)
    << throw_function(func) << throw_file(file) << throw_line(line) );
}

void assertion_failed_msg(char const* expr, char const* msg, char const* func,
                          char const* file, long line)
{
  throw_exception( assert_failure(expr, msg, func, file, line)
    << throw_function(func) << throw_file(file) << throw_line(line) );
}

} // namespace boost

