# Mac Catalyst (macabi) toolchain — used BOTH as the framework build's chainload (via the maccatalyst
# preset's VCPKG_CHAINLOAD_TOOLCHAIN_FILE, which vcpkg.cmake chainloads) AND by vcpkg for the dependency
# builds (the arm64-maccatalyst triplet points VCPKG_CHAINLOAD_TOOLCHAIN_FILE here).
#
# WHY: .NET MAUI's macOS target IS Mac Catalyst (UIKit on macOS), not AppKit. To get visual parity with
# MAUI on a Mac, the port reuses its iOS UIKit handlers (src/platform/ios/*.mm) retargeted to the macabi
# ABI — a NATIVE macOS build (CMAKE_SYSTEM_NAME=Darwin, so the binary runs on this Mac, CMAKE_CROSSCOMPILING
# stays false) whose compiler target points at the macOS SDK's "iOSSupport" unzippered twin (where the
# Catalyst UIKit headers + frameworks live).
#
# The macCatalyst 26.0 floor is REQUIRED, not arbitrary: this SDK's libc++ marks floating-point
# std::from_chars/std::to_chars (used by the graphics parsers + std::format across the C++23 surface)
# "introduced in macCatalyst 26.0" — at ios18.0-macabi they are unavailable and every such TU fails to
# compile. 26.0 mirrors the ios preset's 26.0 deployment floor (same SDK/libc++ availability story).
#
# Idempotent: this file is included multiple times (vcpkg dep builds + the main configure); guard the work.
if(_MAUI_MACCATALYST_TOOLCHAIN_INCLUDED)
  return()
endif()
set(_MAUI_MACCATALYST_TOOLCHAIN_INCLUDED TRUE)

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)

# The Catalyst headers/frameworks live under <macOS SDK>/System/iOSSupport (the unzippered twin).
execute_process(COMMAND xcrun --sdk macosx --show-sdk-path
  OUTPUT_VARIABLE _maui_macos_sdk OUTPUT_STRIP_TRAILING_WHITESPACE)
set(_maui_ioss "${_maui_macos_sdk}/System/iOSSupport")

set(CMAKE_OSX_SYSROOT "${_maui_macos_sdk}" CACHE STRING "macOS SDK (Catalyst builds against it)" FORCE)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)
# Do NOT set CMAKE_OSX_DEPLOYMENT_TARGET: the macabi target triple below carries the platform+version
# (macCatalyst 26.0); a separate -mmacosx-version-min would describe the wrong platform.

set(_maui_macabi_target "arm64-apple-ios26.0-macabi")
set(_maui_macabi_compile
  "-target ${_maui_macabi_target} -isystem ${_maui_ioss}/usr/include -iframework ${_maui_ioss}/System/Library/Frameworks")
set(_maui_macabi_link
  "-target ${_maui_macabi_target} -L${_maui_ioss}/usr/lib -F${_maui_ioss}/System/Library/Frameworks")

foreach(_lang C CXX OBJC OBJCXX)
  string(APPEND CMAKE_${_lang}_FLAGS_INIT " ${_maui_macabi_compile}")
endforeach()
foreach(_kind EXE SHARED MODULE)
  string(APPEND CMAKE_${_kind}_LINKER_FLAGS_INIT " ${_maui_macabi_link}")
endforeach()
