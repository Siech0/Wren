#include <print>
#include <exception>
#include <iostream>

#include <wren/version.hpp>
#include <wren/platform/window.hpp>
#include <wren/foundation/utility/scope_exit.hpp>
#include <wren/rhi/loader.hpp>
#include <wren/rhi/api/enums.hpp>
#include <wren/rhi/api/features.hpp>


auto main(int argc, char *argv[]) -> int {
  try {
    (void)argc;
    (void)argv;

    std::print("Wren Version: {}\n", WREN_VERSION_STRING);

    // --- Backend selection --------------------------------------------------
    std::print("Select a graphics backend:\n");
    std::print("  1 = Vulkan\n");
    std::print("  2 = OpenGL\n");
    std::print("> ");

    int choice{};
    std::cin >> choice;

    wren::rhi::Backend selected{};
    switch (choice) {
      case 1:  selected = wren::rhi::Backend::Vulkan; break;
      case 2:  selected = wren::rhi::Backend::OpenGL; break;
      default:
        std::print(std::cerr, "Invalid selection '{}'. Expected 1 or 2.\n", choice);
        return 1;
    }

    // --- Load backend DLL ---------------------------------------------------
    auto lib_result = wren::rhi::BackendLibrary::load(selected);
    if (!lib_result) {
      std::print(std::cerr, "Failed to load backend: {}\n", lib_result.error());
      return 1;
    }

    auto& backend = *lib_result;
    std::print("Backend loaded: {} (id={})\n",
               wren::rhi::to_string(backend.backend_id()),
               static_cast<int>(backend.backend_id()));

    // --- Create device ------------------------------------------------------
#ifdef NDEBUG
    constexpr auto k_device_flags = wren::rhi::DeviceFlag::None;
#else
    constexpr auto k_device_flags = wren::rhi::DeviceFlag::Debug;
#endif

    const wren::rhi::DeviceDesc desc{
        .flags = k_device_flags,
    };

    auto dev_result = backend.create_device(desc);
    if (!dev_result) {
      std::print(std::cerr, "Failed to create device: {}\n", dev_result.error());
      return 1;
    }

    auto& device = *dev_result;

    // --- Print capabilities -------------------------------------------------
    const auto& caps = device.capabilities();
    std::print("Device created successfully.\n");
    std::print("  API version : {}.{}\n", caps.apiVersionMajor, caps.apiVersionMinor);
    std::print("  Backend     : {}\n",    wren::rhi::to_string(caps.backend));
    std::print("  Max 2D tex  : {}px\n",  caps.limits.maxImageDimension2D);
    std::print("  Max 3D tex  : {}px\n",  caps.limits.maxImageDimension3D);
    std::print("  Max MSAA    : {}x\n",   caps.limits.maxMSAASamples);
    std::print("  UBO align   : {} bytes\n", caps.limits.uniformBufferAlignment);
    std::print("  SSBO align  : {} bytes\n", caps.limits.storageBufferAlignment);

    // --- Window + main loop -------------------------------------------------
    wren::platform::window::init_system();
    const wren::foundation::utility::scope_exit glfw_init_guard{wren::platform::window::deinit_system};

    wren::platform::window window{800, 600, "Renderer"};
    while (window.should_close() == false) {
      window.poll_events();
    }
  } catch (const std::exception &e) {
    std::print(std::cerr, "Critical Error: {}\n", e.what());
    return 1;
  }
}
