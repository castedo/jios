#ifndef CEL_JIOS_PROTOBUF_IJ_HPP
#define CEL_JIOS_PROTOBUF_IJ_HPP

#include <istream>
#include <google/protobuf/message.h>
#include <jios/jin.hpp>

namespace jios {

void merge_proto_type(ijvalue & ij, google::protobuf::Message & pro);

void jios_read(ijvalue & ij, google::protobuf::Message & pro);

} // namespace

#endif

