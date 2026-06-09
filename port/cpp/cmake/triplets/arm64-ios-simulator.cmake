# Overlay triplet for the `ios` preset (wired via VCPKG_OVERLAY_TRIPLETS; shadows vcpkg's community
# triplet of the same name). Identical to the community arm64-ios-simulator, plus a PINNED deployment
# target matching the preset's CMAKE_OSX_DEPLOYMENT_TARGET — without it, vcpkg-built dependencies
# (gtest/benchmark) default to the SDK's own version (26.5), newer than the installed simulator
# runtimes (26.4) and the preset's floor, which spams min-version linker warnings into every link.
# The floor itself is 26.0: this SDK's libc++ marks floating-point std::from_chars/std::to_chars
# (which the graphics parsers and std::format use) "introduced in iOS 26.0", so the C++23 surface the
# port leans on (PROFILE §1) needs a 26.0 minimum. The installed simulator runtimes (26.4) satisfy it.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME iOS)
set(VCPKG_OSX_SYSROOT iphonesimulator)
set(VCPKG_OSX_DEPLOYMENT_TARGET 26.0)
