#ifndef JIOS_JSONC_PARSER_HPP
#define JIOS_JSONC_PARSER_HPP

#include <memory>
#include <jios/istream_ij.hpp>

namespace jios {


// factory function

std::shared_ptr<istream_parser>
    make_jsonc_parser(std::shared_ptr<istream_facade> const&);

std::shared_ptr<ijsource>
    make_jsonc_ijsource(std::shared_ptr<istream_facade> const&);


} // namespace jios

#endif

