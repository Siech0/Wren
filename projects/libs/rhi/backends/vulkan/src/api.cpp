#include <wren/rhi/vulkan/api.hpp>

#include <wren/rhi/api/enums.hpp>
#include <wren/rhi/api/features.hpp>
#include <wren/rhi/vulkan/device.hpp>
#include <wren/rhi/vulkan/instance.hpp>

#include <vulkan/vulkan_raii.hpp>

#include <wren/foundation/system/platform.hpp>

#include <cstring>
#include <optional>

// -------------------------------------------------------------------------------------------------
// Internal device state
//
// Owns everything from the Vulkan context through the logical device in a
// single heap allocation.  Lifetime is managed by vk_create_device /
// vk_destroy_device, which the loader calls through the vtable.
//
// Named DeviceState to match the forward declaration in wren::rhi::DeviceHandle.
// -------------------------------------------------------------------------------------------------
struct wren::rhi::DeviceState {
    vk::raii::Context                               ctx;
    vk::raii::Instance                              instance   {nullptr};
    std::optional<vk::raii::DebugUtilsMessengerEXT> debug_messenger;
    std::optional<wren::rhi::vulkan::VulkanDevice>  device;
    wren::rhi::Capabilities                         capabilities{};
};

// -------------------------------------------------------------------------------------------------
// Function pointer implementations
// -------------------------------------------------------------------------------------------------

static uint8_t vk_backend_id() noexcept {
    return static_cast<uint8_t>(wren::rhi::Backend::Vulkan);
}

static wren::rhi::DeviceHandle vk_create_device(
    wren::rhi::DeviceDesc const* desc,
    char*       err_buf,
    std::size_t err_len) noexcept
{
    auto write_error = [&](const char* msg) noexcept {
        if (err_buf && err_len > 0) {
#ifdef WREN_COMPILER_MSVC_ABI
            strncpy_s(err_buf, err_len, msg, err_len - 1);
#else
            std::strncpy(err_buf, msg, err_len - 1);
            err_buf[err_len - 1] = '\0';
#endif
        }
    };

    if (!desc) {
        write_error("null DeviceDesc pointer");
        return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* state = new (std::nothrow) wren::rhi::DeviceState{};
    if (!state) {
        write_error("out of memory");
        return nullptr;
    }

    try {
        const bool debug = wren::rhi::has_any(desc->flags, wren::rhi::DeviceFlag::Debug);

        wren::rhi::vulkan::InstanceConfig cfg{
            .enable_debug = debug,
        };

        auto inst = wren::rhi::vulkan::create_instance(state->ctx, cfg);
        if (!inst) {
            write_error(inst.error().c_str());
            delete state; // NOLINT
            return nullptr;
        }
        state->instance = std::move(*inst);

        if (debug) {
            state->debug_messenger =
                wren::rhi::vulkan::create_debug_messenger(state->instance);
        }

        auto dev = wren::rhi::vulkan::VulkanDevice::create(state->instance, *desc);
        if (!dev) {
            write_error(dev.error().message.c_str());
            delete state; // NOLINT
            return nullptr;
        }

        state->capabilities = dev->capabilities();
        state->device.emplace(std::move(*dev));

        return state;

    } catch (std::exception const& e) {
        write_error(e.what());
    } catch (...) {
        write_error("unknown exception during device creation");
    }

    delete state; // NOLINT
    return nullptr;
}

static void vk_destroy_device(wren::rhi::DeviceHandle device) noexcept {
    delete device; // NOLINT
}

static void vk_get_capabilities(
    wren::rhi::DeviceHandle  device,
    wren::rhi::Capabilities* out) noexcept
{
    if (device && out) {
        *out = device->capabilities;
    }
}

// -------------------------------------------------------------------------------------------------
// Static backend vtable + DLL entry point
// -------------------------------------------------------------------------------------------------

static wren::rhi::BackendVTable s_vulkan_backend{
    .abi_version      = wren::rhi::k_backend_abi_version,
    .backend_id       = vk_backend_id,
    .create_device    = vk_create_device,
    .destroy_device   = vk_destroy_device,
    .get_capabilities = vk_get_capabilities,
};

extern "C" WREN_RHI_VULKAN_EXPORT wren::rhi::BackendVTable* wren_rhi_create() {
    return &s_vulkan_backend;
}
