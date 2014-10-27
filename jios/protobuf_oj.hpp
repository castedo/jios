#ifndef CEL_JIOS_PROTOBUF_OJ_HPP
#define CEL_JIOS_PROTOBUF_OJ_HPP

#include <ostream>
#include <boost/filesystem/path.hpp>
#include <google/protobuf/message.h>
#include <jios/jout.hpp>

namespace jios {


void jios_write(ojvalue & oj, google::protobuf::Message const& pro);

void print_proto_type(std::ostream & os,
                      google::protobuf::Message const& pro);

void print_proto_type(boost::filesystem::path const& file_path,
                      google::protobuf::Message const& pro);


} // namespace

#endif

