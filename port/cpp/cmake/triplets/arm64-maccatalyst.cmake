# Overlay triplet for the `maccatalyst` preset (wired via VCPKG_OVERLAY_TRIPLETS). Builds the vcpkg
# dependencies (pugixml + gtest/benchmark) for the Mac Catalyst (macabi) ABI through the shared chainload
# toolchain, so they link cleanly into the macabi framework + apps. Mirrors arm64-ios-simulator.cmake, but
# the Catalyst retarget (target triple, iOSSupport paths, macCatalyst 26.0 floor) all comes from the
# chainload toolchain rather than VCPKG_OSX_SYSROOT/DEPLOYMENT_TARGET (Catalyst is a Darwin host build,
# not a CMAKE_SYSTEM_NAME=iOS cross-build).
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../maccatalyst.toolchain.cmake")
