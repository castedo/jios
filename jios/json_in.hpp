#ifndef JIOS_JSON_IN_HPP
#define JIOS_JSON_IN_HPP

#include <memory>
#include <istream>

#include <jios/jin.hpp>

namespace jios {

ijstream json_in(std::istream & is);
ijstream json_in(std::shared_ptr<std::istream> const& p_is);

}

#endif

