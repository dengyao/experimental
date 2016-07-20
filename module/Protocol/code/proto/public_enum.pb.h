// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: proto/public_enum.proto

#ifndef PROTOBUF_proto_2fpublic_5fenum_2eproto__INCLUDED
#define PROTOBUF_proto_2fpublic_5fenum_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
// @@protoc_insertion_point(includes)

namespace pub {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_proto_2fpublic_5fenum_2eproto();
void protobuf_AssignDesc_proto_2fpublic_5fenum_2eproto();
void protobuf_ShutdownFile_proto_2fpublic_5fenum_2eproto();


enum ErrorCode {
  kNotLoggedIn = 10001,
  kDisconnect = 10002,
  kNotConnected = 10003,
  kInvalidProtocol = 10004,
  kInvalidOperation = 10005,
  kNotFoundDatabase = 10006,
  kResourceInsufficiency = 10007,
  kDestinationUnreachable = 10008,
  kInvalidNodeType = 10009,
  kRepeatLogin = 10010,
  kInvalidDataPacket = 10011,
  kDatabaseError = 10012,
  kCreateAccountFailed = 10013,
  kPartitionNotExist = 10014,
  kLinkerNotExist = 10015
};
bool ErrorCode_IsValid(int value);
const ErrorCode ErrorCode_MIN = kNotLoggedIn;
const ErrorCode ErrorCode_MAX = kLinkerNotExist;
const int ErrorCode_ARRAYSIZE = ErrorCode_MAX + 1;

const ::google::protobuf::EnumDescriptor* ErrorCode_descriptor();
inline const ::std::string& ErrorCode_Name(ErrorCode value) {
  return ::google::protobuf::internal::NameOfEnum(
    ErrorCode_descriptor(), value);
}
inline bool ErrorCode_Parse(
    const ::std::string& name, ErrorCode* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ErrorCode>(
    ErrorCode_descriptor(), name, value);
}
// ===================================================================


// ===================================================================


// ===================================================================


// @@protoc_insertion_point(namespace_scope)

}  // namespace pub

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::pub::ErrorCode> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::pub::ErrorCode>() {
  return ::pub::ErrorCode_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_proto_2fpublic_5fenum_2eproto__INCLUDED
