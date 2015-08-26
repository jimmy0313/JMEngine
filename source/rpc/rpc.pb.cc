// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: rpc.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "rpc.pb.h"

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

namespace {

const ::google::protobuf::Descriptor* jme_rpc_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  jme_rpc_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_rpc_2eproto() {
  protobuf_AddDesc_rpc_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "rpc.proto");
  GOOGLE_CHECK(file != NULL);
  jme_rpc_descriptor_ = file->message_type(0);
  static const int jme_rpc_offsets_[3] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(jme_rpc, rpc_id_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(jme_rpc, method_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(jme_rpc, params_),
  };
  jme_rpc_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      jme_rpc_descriptor_,
      jme_rpc::default_instance_,
      jme_rpc_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(jme_rpc, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(jme_rpc, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(jme_rpc));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_rpc_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    jme_rpc_descriptor_, &jme_rpc::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_rpc_2eproto() {
  delete jme_rpc::default_instance_;
  delete jme_rpc_reflection_;
}

void protobuf_AddDesc_rpc_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\trpc.proto\"9\n\007jme_rpc\022\016\n\006rpc_id\030\001 \002(\005\022\016"
    "\n\006method\030\002 \002(\t\022\016\n\006params\030\003 \002(\014", 70);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "rpc.proto", &protobuf_RegisterTypes);
  jme_rpc::default_instance_ = new jme_rpc();
  jme_rpc::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_rpc_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_rpc_2eproto {
  StaticDescriptorInitializer_rpc_2eproto() {
    protobuf_AddDesc_rpc_2eproto();
  }
} static_descriptor_initializer_rpc_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int jme_rpc::kRpcIdFieldNumber;
const int jme_rpc::kMethodFieldNumber;
const int jme_rpc::kParamsFieldNumber;
#endif  // !_MSC_VER

jme_rpc::jme_rpc()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void jme_rpc::InitAsDefaultInstance() {
}

jme_rpc::jme_rpc(const jme_rpc& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void jme_rpc::SharedCtor() {
  _cached_size_ = 0;
  rpc_id_ = 0;
  method_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  params_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

jme_rpc::~jme_rpc() {
  SharedDtor();
}

void jme_rpc::SharedDtor() {
  if (method_ != &::google::protobuf::internal::kEmptyString) {
    delete method_;
  }
  if (params_ != &::google::protobuf::internal::kEmptyString) {
    delete params_;
  }
  if (this != default_instance_) {
  }
}

void jme_rpc::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* jme_rpc::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return jme_rpc_descriptor_;
}

const jme_rpc& jme_rpc::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_rpc_2eproto();
  return *default_instance_;
}

jme_rpc* jme_rpc::default_instance_ = NULL;

jme_rpc* jme_rpc::New() const {
  return new jme_rpc;
}

void jme_rpc::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    rpc_id_ = 0;
    if (has_method()) {
      if (method_ != &::google::protobuf::internal::kEmptyString) {
        method_->clear();
      }
    }
    if (has_params()) {
      if (params_ != &::google::protobuf::internal::kEmptyString) {
        params_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool jme_rpc::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 rpc_id = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &rpc_id_)));
          set_has_rpc_id();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_method;
        break;
      }

      // required string method = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_method:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_method()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->method().data(), this->method().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(26)) goto parse_params;
        break;
      }

      // required bytes params = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_params:
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_params()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void jme_rpc::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required int32 rpc_id = 1;
  if (has_rpc_id()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(1, this->rpc_id(), output);
  }

  // required string method = 2;
  if (has_method()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->method().data(), this->method().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      2, this->method(), output);
  }

  // required bytes params = 3;
  if (has_params()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytes(
      3, this->params(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* jme_rpc::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required int32 rpc_id = 1;
  if (has_rpc_id()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(1, this->rpc_id(), target);
  }

  // required string method = 2;
  if (has_method()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->method().data(), this->method().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->method(), target);
  }

  // required bytes params = 3;
  if (has_params()) {
    target =
      ::google::protobuf::internal::WireFormatLite::WriteBytesToArray(
        3, this->params(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int jme_rpc::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required int32 rpc_id = 1;
    if (has_rpc_id()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->rpc_id());
    }

    // required string method = 2;
    if (has_method()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->method());
    }

    // required bytes params = 3;
    if (has_params()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->params());
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

void jme_rpc::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const jme_rpc* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const jme_rpc*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void jme_rpc::MergeFrom(const jme_rpc& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_rpc_id()) {
      set_rpc_id(from.rpc_id());
    }
    if (from.has_method()) {
      set_method(from.method());
    }
    if (from.has_params()) {
      set_params(from.params());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void jme_rpc::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void jme_rpc::CopyFrom(const jme_rpc& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool jme_rpc::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000007) != 0x00000007) return false;

  return true;
}

void jme_rpc::Swap(jme_rpc* other) {
  if (other != this) {
    std::swap(rpc_id_, other->rpc_id_);
    std::swap(method_, other->method_);
    std::swap(params_, other->params_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata jme_rpc::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = jme_rpc_descriptor_;
  metadata.reflection = jme_rpc_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)