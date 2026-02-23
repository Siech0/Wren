#include <wren/rhi/vulkan/instance.hpp>

#include <cstdint>
#include <cstring>
#include <format>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_raii.hpp>

#include "vk_capabilities.hpp"

namespace wren::rhi::vulkan {

// -------------------------------------------------------------------------------------------------
// Debug messenger callback
// -------------------------------------------------------------------------------------------------
static VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_messenger_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT  severity,
    vk::DebugUtilsMessageTypeFlagsEXT         /*type*/,
    vk::DebugUtilsMessengerCallbackDataEXT const* data,
    void* /*user*/) noexcept
{
    if (!data) return vk::False;

    const std::string_view msg{data->pMessage ? data->pMessage : "(null)"};
    using Sev = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    if      (severity == Sev::eError)   SPDLOG_ERROR("[VK] {}", msg);
    else if (severity == Sev::eWarning) SPDLOG_WARN ("[VK] {}", msg);
    else if (severity == Sev::eInfo)    SPDLOG_INFO ("[VK] {}", msg);
    else                                SPDLOG_TRACE("[VK] {}", msg);
    return vk::False;
}

// -------------------------------------------------------------------------------------------------
// Internal helpers
// -------------------------------------------------------------------------------------------------
namespace {

[[nodiscard]] bool layer_available(
    std::vector<vk::LayerProperties> const& layers,
    const char*                             name) noexcept
{
    return std::ranges::any_of(layers, [name](vk::LayerProperties const& l) {
        return std::strcmp(l.layerName.data(), name) == 0;
    });
}

[[nodiscard]] bool instance_extension_available(
    std::vector<vk::ExtensionProperties> const& exts,
    const char*                                 name) noexcept
{
    return std::ranges::any_of(exts, [name](vk::ExtensionProperties const& e) {
        return std::strcmp(e.extensionName.data(), name) == 0;
    });
}

[[nodiscard]] vk::DebugUtilsMessengerCreateInfoEXT make_debug_create_info() noexcept {
    using Sev  = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

    return vk::DebugUtilsMessengerCreateInfoEXT{}
        .setMessageSeverity(Sev::eError | Sev::eWarning | Sev::eInfo)
        .setMessageType(Type::eGeneral | Type::eValidation | Type::ePerformance)
        .setPfnUserCallback(debug_messenger_callback);
}

} // anonymous namespace

// -------------------------------------------------------------------------------------------------
// create_instance
// -------------------------------------------------------------------------------------------------
auto create_instance(vk::raii::Context& ctx, InstanceConfig const& cfg)
    -> std::expected<vk::raii::Instance, std::string>
{
    try {
        // ------------------------------------------------------------------
        // Enumerate available layers and extensions.
        // ------------------------------------------------------------------
        auto const available_layers = ctx.enumerateInstanceLayerProperties();
        auto const available_exts   = ctx.enumerateInstanceExtensionProperties();

        // ------------------------------------------------------------------
        // Layers: validation (debug only, soft requirement).
        // ------------------------------------------------------------------
        std::vector<const char*> layers;

        if (cfg.enable_debug) {
            constexpr const char* k_validation = "VK_LAYER_KHRONOS_validation";
            if (layer_available(available_layers, k_validation)) {
                layers.push_back(k_validation);
            } else {
                SPDLOG_WARN("[wren/rhi/vulkan] Validation layer '{}' not available; "
                            "continuing without validation.", k_validation);
            }
        }

        // ------------------------------------------------------------------
        // Extensions: debug utils (optional).
        // ------------------------------------------------------------------
        std::vector<const char*> extensions;
        bool debug_utils_available = false;

        if (cfg.enable_debug &&
            instance_extension_available(available_exts, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            debug_utils_available = true;
        }

        // ------------------------------------------------------------------
        // Application info: request Vulkan 1.3 as the minimum API level.
        // ------------------------------------------------------------------
        auto app_info = vk::ApplicationInfo{}
            .setPApplicationName(cfg.application_name.data())
            .setApplicationVersion(cfg.application_version)
            .setPEngineName("wren")
            .setEngineVersion(VK_MAKE_VERSION(0, 1, 0))
            .setApiVersion(VK_API_VERSION_1_3);

        // Attach debug messenger create info to instance creation so that
        // validation covers vkCreateInstance / vkDestroyInstance as well.
        auto debug_info = make_debug_create_info();
        void* p_next = (cfg.enable_debug && debug_utils_available)
            ? static_cast<void*>(&debug_info)
            : nullptr;

        auto create_info = vk::InstanceCreateInfo{}
            .setPNext(p_next)
            .setPApplicationInfo(&app_info)
            .setPEnabledLayerNames(layers)
            .setPEnabledExtensionNames(extensions);

        // ------------------------------------------------------------------
        // Create instance.
        // ------------------------------------------------------------------
        return ctx.createInstance(create_info);

    } catch (vk::SystemError const& err) {
        return std::unexpected{std::format("Vulkan instance creation failed: {}", err.what())};
    } catch (std::exception const& err) {
        return std::unexpected{std::format("Instance creation error: {}", err.what())};
    }
}

// -------------------------------------------------------------------------------------------------
// create_debug_messenger
// -------------------------------------------------------------------------------------------------
auto create_debug_messenger(vk::raii::Instance& instance)
    -> vk::raii::DebugUtilsMessengerEXT
{
    return instance.createDebugUtilsMessengerEXT(make_debug_create_info());
}

// -------------------------------------------------------------------------------------------------
// enumerate_adapters
// -------------------------------------------------------------------------------------------------
auto enumerate_adapters(vk::raii::Instance const& instance) -> std::vector<AdapterInfo> {
    try {
        auto const phys_devices = instance.enumeratePhysicalDevices();
        std::vector<AdapterInfo> result;
        result.reserve(phys_devices.size());

        for (uint32_t i = 0; i < static_cast<uint32_t>(phys_devices.size()); ++i) {
            result.push_back(detail::make_adapter_info(i, phys_devices[i]));
        }

        return result;
    } catch (vk::SystemError const& err) {
        SPDLOG_ERROR("[wren/rhi/vulkan] enumerate_adapters failed: {}", err.what());
        return {};
    }
}

} // namespace wren::rhi::vulkan
