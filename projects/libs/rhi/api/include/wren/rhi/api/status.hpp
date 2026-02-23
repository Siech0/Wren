#pragma once
#include <cstdint>

namespace wren::rhi {

// Uniform status codes for creation/compilation/etc.
// Use at *creation time* so the hot path never checks capabilities.
enum class Status : uint8_t {
  Ok = 0,
  MissingRequiredFeature,
  UnsupportedFormat,
  UnsupportedSampleCount,
  UnsupportedQueueType,
  UnsupportedLimit,
  OutOfMemory,
  InvalidArgument,
  InternalError
};

inline const char* to_string(Status s) {
  switch (s) {
    case Status::Ok:                     return "Ok";
    case Status::MissingRequiredFeature: return "MissingRequiredFeature";
    case Status::UnsupportedFormat:      return "UnsupportedFormat";
    case Status::UnsupportedSampleCount: return "UnsupportedSampleCount";
    case Status::UnsupportedQueueType:   return "UnsupportedQueueType";
    case Status::UnsupportedLimit:       return "UnsupportedLimit";
    case Status::OutOfMemory:            return "OutOfMemory";
    case Status::InvalidArgument:        return "InvalidArgument";
    default:                             return "InternalError";
  }
}

} // namespace wren::rhi
