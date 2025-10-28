#ifndef WREN_RHI_API_API_HPP
#define WREN_RHI_API_API_HPP


#include <wren/rhi/api/enums.hpp>
#include <wren/rhi/api/features.hpp>

namespace wren::rhi {

// -------------------------------------------------------------------------------------------------
// Status / Error / Result
//
// Contract:
//   - Factories must never return “success” with a half-valid device.
//   - If creation fails, set 'error.code' and 'error.message' to something the user can act on.
//   - Keep messages short; include a 'detail' string if you want backend-specific context.
//
// Common codes:
//   MissingRequiredFeature  → a bit from DeviceFeatureRequest::required isn’t supported
//   UnsupportedFormat       → e.g., storage usage with a format the device can’t support as UAV
//   UnsupportedSampleCount  → requested MSAA exceeds device/format capability
//   UnsupportedQueueType    → (rare) requested queue family/type isn’t available
//   UnsupportedLimit        → exceeds numeric Limits (e.g., too many color attachments)
//   OutOfMemory             → native allocation failed
//   InvalidArgument         → null/zero/contradictory inputs at creation time
//   InternalError           → unexpected backend error (include detail string)
// -------------------------------------------------------------------------------------------------
enum class Status {
  Ok = 0,
  MissingRequiredFeature,
  UnsupportedFormat,
  UnsupportedSampleCount,
  UnsupportedQueueType,
  UnsupportedLimit,      // e.g., attribute count > limit
  OutOfMemory,
  InvalidArgument,
  InternalError
};

struct Error {
  Status       code     = Status::Ok;       // non-Ok indicates failure
  const char*  message  = nullptr;          // short, user-facing reason
  const char*  detail   = nullptr;          // optional backend-specific detail
};


} // namespace wren::rhi

#endif // WREN_RHI_API_API_HPP
