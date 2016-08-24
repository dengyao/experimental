// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: proto/public_struct.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "proto/public_struct.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace pub {

namespace {

const ::google::protobuf::Descriptor* PingReq_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  PingReq_reflection_ = NULL;
const ::google::protobuf::Descriptor* PongRsp_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  PongRsp_reflection_ = NULL;
const ::google::protobuf::Descriptor* ErrorRsp_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  ErrorRsp_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_proto_2fpublic_5fstruct_2eproto() {
  protobuf_AddDesc_proto_2fpublic_5fstruct_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "proto/public_struct.proto");
  GOOGLE_CHECK(file != NULL);
  PingReq_descriptor_ = file->message_type(0);
  static const int PingReq_offsets_[1] = {
  };
  PingReq_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      PingReq_descriptor_,
      PingReq::default_instance_,
      PingReq_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PingReq, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PingReq, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(PingReq));
  PongRsp_descriptor_ = file->message_type(1);
  static const int PongRsp_offsets_[1] = {
  };
  PongRsp_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      PongRsp_descriptor_,
      PongRsp::default_instance_,
      PongRsp_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PongRsp, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PongRsp, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(PongRsp));
  ErrorRsp_descriptor_ = file->message_type(2);
  static const int ErrorRsp_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ErrorRsp, error_code_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ErrorRsp, what_),
  };
  ErrorRsp_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      ErrorRsp_descriptor_,
      ErrorRsp::default_instance_,
      ErrorRsp_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ErrorRsp, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ErrorRsp, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(ErrorRsp));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_proto_2fpublic_5fstruct_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    PingReq_descriptor_, &PingReq::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    PongRsp_descriptor_, &PongRsp::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    ErrorRsp_descriptor_, &ErrorRsp::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_proto_2fpublic_5fstruct_2eproto() {
  delete PingReq::default_instance_;
  delete PingReq_reflection_;
  delete PongRsp::default_instance_;
  delete PongRsp_reflection_;
  delete ErrorRsp::default_instance_;
  delete ErrorRsp_reflection_;
}

void protobuf_AddDesc_proto_2fpublic_5fstruct_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::pub::protobuf_AddDesc_proto_2fpublic_5fenum_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\031proto/public_struct.proto\022\003pub\032\027proto/"
    "public_enum.proto\"\t\n\007PingReq\"\t\n\007PongRsp\""
    "<\n\010ErrorRsp\022\"\n\nerror_code\030\001 \002(\0162\016.pub.Er"
    "rorCode\022\014\n\004what\030\002 \001(\t", 141);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "proto/public_struct.proto", &protobuf_RegisterTypes);
  PingReq::default_instance_ = new PingReq();
  PongRsp::default_instance_ = new PongRsp();
  ErrorRsp::default_instance_ = new ErrorRsp();
  PingReq::default_instance_->InitAsDefaultInstance();
  PongRsp::default_instance_->InitAsDefaultInstance();
  ErrorRsp::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_proto_2fpublic_5fstruct_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_proto_2fpublic_5fstruct_2eproto {
  StaticDescriptorInitializer_proto_2fpublic_5fstruct_2eproto() {
    protobuf_AddDesc_proto_2fpublic_5fstruct_2eproto();
  }
} static_descriptor_initializer_proto_2fpublic_5fstruct_2eproto_;

// ===================================================================

#ifndef _MSC_VER
#endif  // !_MSC_VER

PingReq::PingReq()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:pub.PingReq)
}

void PingReq::InitAsDefaultInstance() {
}

PingReq::PingReq(const PingReq& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:pub.PingReq)
}

void PingReq::SharedCtor() {
  _cached_size_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

PingReq::~PingReq() {
  // @@protoc_insertion_point(destructor:pub.PingReq)
  SharedDtor();
}

void PingReq::SharedDtor() {
  if (this != default_instance_) {
  }
}

void PingReq::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* PingReq::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return PingReq_descriptor_;
}

const PingReq& PingReq::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_proto_2fpublic_5fstruct_2eproto();
  return *default_instance_;
}

PingReq* PingReq::default_instance_ = NULL;

PingReq* PingReq::New() const {
  return new PingReq;
}

void PingReq::Clear() {
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool PingReq::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:pub.PingReq)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
  handle_unusual:
    if (tag == 0 ||
        ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
        ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
      goto success;
    }
    DO_(::google::protobuf::internal::WireFormat::SkipField(
          input, tag, mutable_unknown_fields()));
  }
success:
  // @@protoc_insertion_point(parse_success:pub.PingReq)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:pub.PingReq)
  return false;
#undef DO_
}

void PingReq::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:pub.PingReq)
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:pub.PingReq)
}

::google::protobuf::uint8* PingReq::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:pub.PingReq)
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:pub.PingReq)
  return target;
}

int PingReq::ByteSize() const {
  int total_size = 0;

  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void PingReq::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const PingReq* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const PingReq*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void PingReq::MergeFrom(const PingReq& from) {
  GOOGLE_CHECK_NE(&from, this);
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void PingReq::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void PingReq::CopyFrom(const PingReq& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PingReq::IsInitialized() const {

  return true;
}

void PingReq::Swap(PingReq* other) {
  if (other != this) {
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata PingReq::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = PingReq_descriptor_;
  metadata.reflection = PingReq_reflection_;
  return metadata;
}


// ===================================================================

#ifndef _MSC_VER
#endif  // !_MSC_VER

PongRsp::PongRsp()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:pub.PongRsp)
}

void PongRsp::InitAsDefaultInstance() {
}

PongRsp::PongRsp(const PongRsp& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:pub.PongRsp)
}

void PongRsp::SharedCtor() {
  _cached_size_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

PongRsp::~PongRsp() {
  // @@protoc_insertion_point(destructor:pub.PongRsp)
  SharedDtor();
}

void PongRsp::SharedDtor() {
  if (this != default_instance_) {
  }
}

void PongRsp::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* PongRsp::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return PongRsp_descriptor_;
}

const PongRsp& PongRsp::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_proto_2fpublic_5fstruct_2eproto();
  return *default_instance_;
}

PongRsp* PongRsp::default_instance_ = NULL;

PongRsp* PongRsp::New() const {
  return new PongRsp;
}

void PongRsp::Clear() {
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool PongRsp::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:pub.PongRsp)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
  handle_unusual:
    if (tag == 0 ||
        ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
        ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
      goto success;
    }
    DO_(::google::protobuf::internal::WireFormat::SkipField(
          input, tag, mutable_unknown_fields()));
  }
success:
  // @@protoc_insertion_point(parse_success:pub.PongRsp)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:pub.PongRsp)
  return false;
#undef DO_
}

void PongRsp::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:pub.PongRsp)
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:pub.PongRsp)
}

::google::protobuf::uint8* PongRsp::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:pub.PongRsp)
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:pub.PongRsp)
  return target;
}

int PongRsp::ByteSize() const {
  int total_size = 0;

  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void PongRsp::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const PongRsp* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const PongRsp*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void PongRsp::MergeFrom(const PongRsp& from) {
  GOOGLE_CHECK_NE(&from, this);
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void PongRsp::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void PongRsp::CopyFrom(const PongRsp& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PongRsp::IsInitialized() const {

  return true;
}

void PongRsp::Swap(PongRsp* other) {
  if (other != this) {
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata PongRsp::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = PongRsp_descriptor_;
  metadata.reflection = PongRsp_reflection_;
  return metadata;
}


// ===================================================================

#ifndef _MSC_VER
const int ErrorRsp::kErrorCodeFieldNumber;
const int ErrorRsp::kWhatFieldNumber;
#endif  // !_MSC_VER

ErrorRsp::ErrorRsp()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:pub.ErrorRsp)
}

void ErrorRsp::InitAsDefaultInstance() {
}

ErrorRsp::ErrorRsp(const ErrorRsp& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:pub.ErrorRsp)
}

void ErrorRsp::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  error_code_ = 10001;
  what_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

ErrorRsp::~ErrorRsp() {
  // @@protoc_insertion_point(destructor:pub.ErrorRsp)
  SharedDtor();
}

void ErrorRsp::SharedDtor() {
  if (what_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete what_;
  }
  if (this != default_instance_) {
  }
}

void ErrorRsp::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* ErrorRsp::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return ErrorRsp_descriptor_;
}

const ErrorRsp& ErrorRsp::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_proto_2fpublic_5fstruct_2eproto();
  return *default_instance_;
}

ErrorRsp* ErrorRsp::default_instance_ = NULL;

ErrorRsp* ErrorRsp::New() const {
  return new ErrorRsp;
}

void ErrorRsp::Clear() {
  if (_has_bits_[0 / 32] & 3) {
    error_code_ = 10001;
    if (has_what()) {
      if (what_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        what_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool ErrorRsp::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:pub.ErrorRsp)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required .pub.ErrorCode error_code = 1;
      case 1: {
        if (tag == 8) {
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::pub::ErrorCode_IsValid(value)) {
            set_error_code(static_cast< ::pub::ErrorCode >(value));
          } else {
            mutable_unknown_fields()->AddVarint(1, value);
          }
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(18)) goto parse_what;
        break;
      }

      // optional string what = 2;
      case 2: {
        if (tag == 18) {
         parse_what:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_what()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->what().data(), this->what().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "what");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:pub.ErrorRsp)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:pub.ErrorRsp)
  return false;
#undef DO_
}

void ErrorRsp::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:pub.ErrorRsp)
  // required .pub.ErrorCode error_code = 1;
  if (has_error_code()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      1, this->error_code(), output);
  }

  // optional string what = 2;
  if (has_what()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->what().data(), this->what().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "what");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->what(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:pub.ErrorRsp)
}

::google::protobuf::uint8* ErrorRsp::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:pub.ErrorRsp)
  // required .pub.ErrorCode error_code = 1;
  if (has_error_code()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      1, this->error_code(), target);
  }

  // optional string what = 2;
  if (has_what()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->what().data(), this->what().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "what");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->what(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:pub.ErrorRsp)
  return target;
}

int ErrorRsp::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required .pub.ErrorCode error_code = 1;
    if (has_error_code()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->error_code());
    }

    // optional string what = 2;
    if (has_what()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->what());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void ErrorRsp::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const ErrorRsp* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const ErrorRsp*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void ErrorRsp::MergeFrom(const ErrorRsp& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_error_code()) {
      set_error_code(from.error_code());
    }
    if (from.has_what()) {
      set_what(from.what());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void ErrorRsp::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void ErrorRsp::CopyFrom(const ErrorRsp& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ErrorRsp::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;

  return true;
}

void ErrorRsp::Swap(ErrorRsp* other) {
  if (other != this) {
    std::swap(error_code_, other->error_code_);
    std::swap(what_, other->what_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata ErrorRsp::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = ErrorRsp_descriptor_;
  metadata.reflection = ErrorRsp_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace pub

// @@protoc_insertion_point(global_scope)