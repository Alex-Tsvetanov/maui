# WinUI 3 feasibility probe (see main.cpp). Deliberately standalone: it answers "does the toolchain
# work" before any of the port's 400-file build is involved, so a failure here is unambiguous.
#
# MAUI_WINUI_GENERATED / MAUI_WINAPPSDK are supplied by build_winui_probe.ps1, which restores the NuGet
# packages and runs cppwinrt to produce the projection headers. CMake does not know about NuGet, and
# teaching it would add a moving part for no benefit while the question is still "does this compile".
cmake_minimum_required(VERSION 3.24)
project(maui_winui_probe CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MAUI_WINUI_GENERATED)
  message(FATAL_ERROR "MAUI_WINUI_GENERATED not set (path to the cppwinrt-generated headers). "
                      "Run tools/parity/windows/build_winui_probe.ps1 instead of invoking cmake directly.")
endif()
if(NOT MAUI_WINAPPSDK)
  message(FATAL_ERROR "MAUI_WINAPPSDK not set (path to the restored Microsoft.WindowsAppSDK package).")
endif()

# The Windows App SDK ships per-RID import libs; pick the one matching THIS BUILD's ABI. Driven by
# MAUI_TARGET_ABI from build_winui_probe.ps1 rather than CMAKE_SYSTEM_PROCESSOR: on an ARM64 host
# compiling for x64 (Hostarm64\x64, which is what VC.Tools.x86.x64 gives you) CMAKE_SYSTEM_PROCESSOR
# still reports ARM64, so keying off it would link the wrong import library.
if(NOT MAUI_TARGET_ABI)
  set(MAUI_TARGET_ABI "arm64")
endif()
if(MAUI_TARGET_ABI STREQUAL "arm64")
  set(_wasdk_rid "win-arm64")
  set(_wasdk_rid_legacy "win10-arm64")
else()
  set(_wasdk_rid "win-x64")
  set(_wasdk_rid_legacy "win10-x64")
endif()

# The RID folder name changed across Windows App SDK versions, so probe both rather than hardcode.
find_library(MAUI_WASDK_BOOTSTRAP
  NAMES Microsoft.WindowsAppRuntime.Bootstrap
  HINTS "${MAUI_WINAPPSDK}/lib/${_wasdk_rid}" "${MAUI_WINAPPSDK}/lib/${_wasdk_rid_legacy}"
  NO_DEFAULT_PATH)
if(NOT MAUI_WASDK_BOOTSTRAP)
  message(FATAL_ERROR "Microsoft.WindowsAppRuntime.Bootstrap.lib not found under ${MAUI_WINAPPSDK}/lib "
                      "(looked for ${_wasdk_rid} and ${_wasdk_rid_legacy})")
endif()

add_executable(maui_winui_probe WIN32 main.cpp)

target_include_directories(maui_winui_probe PRIVATE
  "${MAUI_WINUI_GENERATED}"        # cppwinrt projection headers (winrt/...)
  "${MAUI_WINAPPSDK}/include")     # MddBootstrap.h

target_link_libraries(maui_winui_probe PRIVATE
  "${MAUI_WASDK_BOOTSTRAP}"
  WindowsApp.lib)                  # the WinRT API surface

target_compile_definitions(maui_winui_probe PRIVATE
  WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE
  # C++/WinRT's coroutine support and the "no Windows.h in the projection" guard.
  WINRT_LEAN_AND_MEAN)

if(MSVC)
  # /await is needed by C++/WinRT's coroutine plumbing on C++20; /bigobj because the projection headers
  # generate very large object files; /permissive- because the projection assumes conforming lookup.
  target_compile_options(maui_winui_probe PRIVATE /EHsc /permissive- /bigobj /await)
  # Subsystem WINDOWS with the wide entry point (wWinMain).
  target_link_options(maui_winui_probe PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup)
endif()

# The .lib above is only an IMPORT library. The actual Microsoft.WindowsAppRuntime.Bootstrap.dll must sit
# beside the exe or the process dies at load with 0xC0000135 STATUS_DLL_NOT_FOUND -- and because that
# happens before main(), there is no error dialog and no log line: the E2E runner sees only
# "process exited early". The runtimes/<rid>/native path carries one DLL per RID, so it is selected by
# the ABI we actually built (see MAUI_TARGET_ABI above), not by the host architecture.
set(_wasdk_boot_dll "${MAUI_WINAPPSDK}/runtimes/${_wasdk_rid}/native/Microsoft.WindowsAppRuntime.Bootstrap.dll")
if(NOT EXISTS "${_wasdk_boot_dll}")
  set(_wasdk_boot_dll "${MAUI_WINAPPSDK}/runtimes/${_wasdk_rid_legacy}/native/Microsoft.WindowsAppRuntime.Bootstrap.dll")
endif()
if(NOT EXISTS "${_wasdk_boot_dll}")
  message(FATAL_ERROR "Microsoft.WindowsAppRuntime.Bootstrap.dll not found for ABI ${MAUI_TARGET_ABI} "
                      "(looked under ${MAUI_WINAPPSDK}/runtimes/{${_wasdk_rid},${_wasdk_rid_legacy}}/native)")
endif()
add_custom_command(TARGET maui_winui_probe POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_wasdk_boot_dll}" "$<TARGET_FILE_DIR:maui_winui_probe>"
  COMMENT "copying Microsoft.WindowsAppRuntime.Bootstrap.dll next to the exe")
