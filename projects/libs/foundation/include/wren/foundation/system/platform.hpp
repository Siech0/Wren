#pragma once

// =================================================================================================
// wren/platform/platform.hpp — compile-time platform, architecture, compiler, and build detection
//
// All macros are defined to 1 when the condition holds, and left UNDEFINED otherwise.
// This means both  #ifdef WREN_PLATFORM_WINDOWS  and  #if WREN_PLATFORM_WINDOWS  work correctly.
//
// Macro families
// ─────────────────────────────────────────────────────────────────────────────
//   WREN_PLATFORM_*   — operating system / runtime environment
//   WREN_ARCH_*       — CPU instruction-set architecture
//   WREN_COMPILER_*   — compiler toolchain
//   WREN_ENDIAN_*     — byte order
//   WREN_BUILD_*      — build configuration
// =================================================================================================

// -------------------------------------------------------------------------------------------------
// Operating system / environment
// -------------------------------------------------------------------------------------------------

// Android must be tested before Linux because Android defines both __ANDROID__ and __linux__.
#if defined(__ANDROID__)
#   define WREN_PLATFORM_ANDROID 1
#   define WREN_PLATFORM_POSIX   1

#elif defined(__EMSCRIPTEN__)
#   define WREN_PLATFORM_EMSCRIPTEN 1

#elif defined(_WIN32)
//  _WIN32 is defined for both 32-bit and 64-bit Windows builds.
#   define WREN_PLATFORM_WINDOWS 1
#   ifdef _WIN64
#       define WREN_PLATFORM_WINDOWS_64 1
#   else
#       define WREN_PLATFORM_WINDOWS_32 1
#   endif

#elif defined(__APPLE__)
#   include <TargetConditionals.h>
#   define WREN_PLATFORM_APPLE 1
#   define WREN_PLATFORM_POSIX 1
#   if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#       define WREN_PLATFORM_IOS 1
#   elif defined(TARGET_OS_TV) && TARGET_OS_TV
#       define WREN_PLATFORM_TVOS 1
#   elif defined(TARGET_OS_WATCH) && TARGET_OS_WATCH
#       define WREN_PLATFORM_WATCHOS 1
#   else
#       define WREN_PLATFORM_MACOS 1
#   endif

#elif defined(__FreeBSD__)
#   define WREN_PLATFORM_FREEBSD 1
#   define WREN_PLATFORM_BSD     1
#   define WREN_PLATFORM_POSIX   1

#elif defined(__NetBSD__)
#   define WREN_PLATFORM_NETBSD 1
#   define WREN_PLATFORM_BSD    1
#   define WREN_PLATFORM_POSIX  1

#elif defined(__OpenBSD__)
#   define WREN_PLATFORM_OPENBSD 1
#   define WREN_PLATFORM_BSD     1
#   define WREN_PLATFORM_POSIX   1

#elif defined(__DragonFly__)
#   define WREN_PLATFORM_DRAGONFLYBSD 1
#   define WREN_PLATFORM_BSD          1
#   define WREN_PLATFORM_POSIX        1

#elif defined(__linux__)
#   define WREN_PLATFORM_LINUX 1
#   define WREN_PLATFORM_POSIX 1

#else
#   error "wren/platform/platform.hpp: unrecognised target platform"
#endif

// Convenience groups
#if defined(WREN_PLATFORM_WINDOWS) || defined(WREN_PLATFORM_MACOS) || defined(WREN_PLATFORM_LINUX)
#   define WREN_PLATFORM_DESKTOP 1
#endif

#if defined(WREN_PLATFORM_IOS) || defined(WREN_PLATFORM_ANDROID)
#   define WREN_PLATFORM_MOBILE 1
#endif

// -------------------------------------------------------------------------------------------------
// CPU architecture
// -------------------------------------------------------------------------------------------------

#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
#   define WREN_ARCH_X86_64 1

#elif defined(_M_IX86) || defined(__i386__)
#   define WREN_ARCH_X86 1

#elif defined(_M_ARM64) || defined(__aarch64__)
#   define WREN_ARCH_ARM64 1

#elif defined(_M_ARM) || defined(__arm__)
#   define WREN_ARCH_ARM32 1

#elif defined(__wasm__) || defined(__EMSCRIPTEN__)
#   define WREN_ARCH_WASM 1

#else
#   error "wren/platform/platform.hpp: unrecognised target architecture"
#endif

// -------------------------------------------------------------------------------------------------
// Compiler
//
// clang-cl (Clang targeting the MSVC ABI) defines both __clang__ and _MSC_VER.
// Test __clang__ first so WREN_COMPILER_CLANG is set in both cases, then
// refine with WREN_COMPILER_CLANG_CL / WREN_COMPILER_MSVC.
// -------------------------------------------------------------------------------------------------

#if defined(__clang__)
#   define WREN_COMPILER_CLANG 1
#   if defined(_MSC_VER)
//      clang-cl: Clang front-end, MSVC ABI and CRT.
#       define WREN_COMPILER_CLANG_CL 1
//      _MSC_VER version exposed to callers even under clang-cl.
#       define WREN_COMPILER_MSVC_VERSION _MSC_VER
#   endif
#   define WREN_COMPILER_CLANG_VERSION \
        (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)

#elif defined(_MSC_VER)
#   define WREN_COMPILER_MSVC         1
#   define WREN_COMPILER_MSVC_VERSION _MSC_VER

#elif defined(__GNUC__)
#   define WREN_COMPILER_GCC 1
#   define WREN_COMPILER_GCC_VERSION \
        (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#else
#   error "wren/platform/platform.hpp: unrecognised compiler"
#endif

// True when the build targets the MSVC ABI and CRT (cl.exe or clang-cl).
#if defined(WREN_COMPILER_MSVC) || defined(WREN_COMPILER_CLANG_CL)
#   define WREN_COMPILER_MSVC_ABI 1
#endif

// -------------------------------------------------------------------------------------------------
// Byte order / endianness
// -------------------------------------------------------------------------------------------------

#if defined(__BYTE_ORDER__)
#   if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#       define WREN_ENDIAN_LITTLE 1
#   elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#       define WREN_ENDIAN_BIG 1
#   else
#       error "wren/platform/platform.hpp: mixed-endian targets are not supported"
#   endif
#elif defined(WREN_PLATFORM_WINDOWS) || defined(WREN_ARCH_X86) || defined(WREN_ARCH_X86_64)
//  All currently supported Windows/x86 targets are little-endian.
#   define WREN_ENDIAN_LITTLE 1
#else
#   error "wren/platform/platform.hpp: unable to determine byte order; define WREN_ENDIAN_LITTLE or WREN_ENDIAN_BIG manually"
#endif

// -------------------------------------------------------------------------------------------------
// Build configuration
// -------------------------------------------------------------------------------------------------

#ifdef NDEBUG
#   define WREN_BUILD_RELEASE 1
#else
#   define WREN_BUILD_DEBUG 1
#endif
