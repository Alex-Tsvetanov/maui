# windows_appsdk.cmake — Windows App SDK (WinUI 3) consumption for the `windows` backend, from plain
# CMake + Ninja (no MSBuild, no NuGet restore at build time).
#
# The lane consumes the ALREADY-CACHED NuGet packages under %USERPROFILE%\.nuget\packages (the
# machine restored them via the .NET MAUI workload / prior WinUI builds; versions pinned below are
# the ones empirically verified on 2026-07-02: projection generated, compiled with LLVM clang 22
# targeting the MSVC ABI, linked with lld-link, and a live WinUI 3 window launched against the
# installed Windows App Runtime 1.8). Three artifacts come out of here:
#
#   1. The C++/WinRT projection headers, generated at configure time by cppwinrt.exe from the
#      .winmd metadata inside the WinAppSDK sub-packages (+ the Windows SDK contracts + the WebView2
#      winmd, which Microsoft.UI.Xaml.winmd hard-references) into build/<preset>/winrt_projection.
#   2. An INTERFACE target `maui_windows_appsdk` carrying the include dirs (projection + MddBootstrap
#      + WindowsAppSDK-VersionInfo headers), the bootstrap import lib, and the projection-induced
#      warning suppression (-Wno-nontrivial-memcall: winrt/base.h com_array memset, benign).
#   3. maui_windows_deploy(<target>): copies the two files an UNPACKAGED WinUI 3 exe must carry
#      beside itself — Microsoft.WindowsAppRuntime.Bootstrap.dll (app-local by design) and the
#      framework resources.pri (without it XamlControlsResources dies at startup resolving
#      ms-appx:///Microsoft.UI.Xaml/Themes/themeresources.xaml, exit 0xC000027B).
#
# Version pins (override with -D if the cache moves): the 1.8 stable line — the installed
# Windows App Runtime 1.8 (8000.879.x) satisfies it. The 2.0 packages are also cached; switching
# the pins is the only change needed to retarget.

if(NOT WIN32)
  message(FATAL_ERROR "windows_appsdk.cmake is Windows-host only (MAUI_BACKEND=windows)")
endif()

set(MAUI_NUGET_CACHE "$ENV{USERPROFILE}/.nuget/packages" CACHE PATH "NuGet global-packages cache")
set(MAUI_WINAPPSDK_WINUI_VER "1.8.251105000" CACHE STRING "Microsoft.WindowsAppSDK.WinUI version")
set(MAUI_WINAPPSDK_FOUNDATION_VER "1.8.251104000" CACHE STRING "Microsoft.WindowsAppSDK.Foundation version")
set(MAUI_WINAPPSDK_IX_VER "1.8.251104001" CACHE STRING "Microsoft.WindowsAppSDK.InteractiveExperiences version")
set(MAUI_WINAPPSDK_RUNTIME_VER "1.8.251106002" CACHE STRING "Microsoft.WindowsAppSDK.Runtime version")
set(MAUI_CPPWINRT_VER "2.0.250303.1" CACHE STRING "Microsoft.Windows.CppWinRT version")
set(MAUI_WEBVIEW2_VER "1.0.3179.45" CACHE STRING "Microsoft.Web.WebView2 version (pinned by the WinUI nuspec)")
set(MAUI_WINDOWS_SDK_VER "10.0.26100.0" CACHE STRING "Windows SDK contract metadata version for cppwinrt -input")

set(_winui_dir "${MAUI_NUGET_CACHE}/microsoft.windowsappsdk.winui/${MAUI_WINAPPSDK_WINUI_VER}")
set(_foundation_dir "${MAUI_NUGET_CACHE}/microsoft.windowsappsdk.foundation/${MAUI_WINAPPSDK_FOUNDATION_VER}")
set(_ix_dir "${MAUI_NUGET_CACHE}/microsoft.windowsappsdk.interactiveexperiences/${MAUI_WINAPPSDK_IX_VER}")
set(_runtime_dir "${MAUI_NUGET_CACHE}/microsoft.windowsappsdk.runtime/${MAUI_WINAPPSDK_RUNTIME_VER}")
set(_cppwinrt_exe "${MAUI_NUGET_CACHE}/microsoft.windows.cppwinrt/${MAUI_CPPWINRT_VER}/bin/cppwinrt.exe")
set(_webview2_winmd "${MAUI_NUGET_CACHE}/microsoft.web.webview2/${MAUI_WEBVIEW2_VER}/lib/Microsoft.Web.WebView2.Core.winmd")

foreach(_probe IN ITEMS
    "${_winui_dir}/metadata/Microsoft.UI.Xaml.winmd"
    "${_foundation_dir}/include/MddBootstrap.h"
    "${_foundation_dir}/lib/native/x64/Microsoft.WindowsAppRuntime.Bootstrap.lib"
    "${_foundation_dir}/runtimes/win-x64/native/Microsoft.WindowsAppRuntime.Bootstrap.dll"
    "${_ix_dir}/metadata/10.0.18362.0/Microsoft.UI.winmd"
    "${_runtime_dir}/include/WindowsAppSDK-VersionInfo.h"
    "${_cppwinrt_exe}"
    "${_webview2_winmd}")
  if(NOT EXISTS "${_probe}")
    message(FATAL_ERROR "Windows App SDK lane: missing '${_probe}'. Restore the pinned NuGet packages "
      "(build any WinAppSDK ${MAUI_WINAPPSDK_WINUI_VER}-era project once, or adjust the MAUI_WINAPPSDK_*_VER pins).")
  endif()
endforeach()

# ---- (1) Generate the C++/WinRT projection at configure time (idempotent; ~3.5 s cold). The stamp
# records the input pins so a version bump regenerates. ----
set(MAUI_WINRT_PROJECTION_DIR "${CMAKE_BINARY_DIR}/winrt_projection")
set(_projection_stamp "${MAUI_WINRT_PROJECTION_DIR}/.stamp-${MAUI_WINAPPSDK_WINUI_VER}-${MAUI_WINAPPSDK_FOUNDATION_VER}-${MAUI_WINAPPSDK_IX_VER}-${MAUI_CPPWINRT_VER}-${MAUI_WINDOWS_SDK_VER}")
if(NOT EXISTS "${_projection_stamp}")
  message(STATUS "windows_appsdk: generating C++/WinRT projection into ${MAUI_WINRT_PROJECTION_DIR}")
  execute_process(
    COMMAND "${_cppwinrt_exe}"
      -input "${MAUI_WINDOWS_SDK_VER}"
      -input "${_winui_dir}/metadata"
      -input "${_foundation_dir}/metadata"
      -input "${_ix_dir}/metadata/10.0.18362.0"
      -input "${_webview2_winmd}"
      -output "${MAUI_WINRT_PROJECTION_DIR}"
    RESULT_VARIABLE _cppwinrt_result
    OUTPUT_VARIABLE _cppwinrt_out
    ERROR_VARIABLE _cppwinrt_err)
  if(NOT _cppwinrt_result EQUAL 0)
    message(FATAL_ERROR "cppwinrt.exe projection generation failed (${_cppwinrt_result}):\n${_cppwinrt_out}\n${_cppwinrt_err}")
  endif()
  file(WRITE "${_projection_stamp}" "ok\n")
endif()

# ---- (2) The consumption surface. INTERFACE (header/lib carrying) — linked PUBLIC into maui_core on
# the windows lane so every TU of the build (handlers, hosting, tests, examples) sees the projection. ----
add_library(maui_windows_appsdk INTERFACE)
# BUILD_INTERFACE-scoped: the projection + SDK headers are a framework-internal concern (no consumer
# TU includes winrt), and the target rides the mauiTargets export set — raw build-dir/NuGet-cache
# include paths would make install(EXPORT) refuse it.
target_include_directories(maui_windows_appsdk INTERFACE
  $<BUILD_INTERFACE:${MAUI_WINRT_PROJECTION_DIR}>
  $<BUILD_INTERFACE:${_foundation_dir}/include>
  $<BUILD_INTERFACE:${_runtime_dir}/include>
  $<BUILD_INTERFACE:${_winui_dir}/include>
  $<BUILD_INTERFACE:${_ix_dir}/include>)
target_link_libraries(maui_windows_appsdk INTERFACE
  "${_foundation_dir}/lib/native/x64/Microsoft.WindowsAppRuntime.Bootstrap.lib"
  WindowsApp.lib
  user32.lib)
# winrt/base.h memsets com_array<T> (benign, upstream-known); RuntimeClass* macro redefinitions come
# from combining windows.h-family headers with the projection in one TU.
target_compile_options(maui_windows_appsdk INTERFACE -Wno-nontrivial-memcall)

# ---- (3) Unpackaged-app deployment: Bootstrap.dll app-local + the framework resources.pri. The pri
# is extracted ONCE from the runtime MSIX (a zip) in the NuGet cache — build-machine stable, no
# dependency on the installed runtime's WindowsApps ACLs. ----
set(_bootstrap_dll "${_foundation_dir}/runtimes/win-x64/native/Microsoft.WindowsAppRuntime.Bootstrap.dll")
set(_pri_extract_dir "${CMAKE_BINARY_DIR}/winappsdk_pri")
if(NOT EXISTS "${_pri_extract_dir}/resources.pri")
  file(GLOB _runtime_msix "${_runtime_dir}/tools/MSIX/win10-x64/Microsoft.WindowsAppRuntime.*.msix")
  list(LENGTH _runtime_msix _msix_count)
  if(_msix_count EQUAL 0)
    message(FATAL_ERROR "windows_appsdk: no runtime MSIX under ${_runtime_dir}/tools/MSIX/win10-x64")
  endif()
  list(GET _runtime_msix 0 _runtime_msix_file)
  message(STATUS "windows_appsdk: extracting resources.pri from ${_runtime_msix_file}")
  file(ARCHIVE_EXTRACT INPUT "${_runtime_msix_file}" DESTINATION "${_pri_extract_dir}" PATTERNS resources.pri)
  if(NOT EXISTS "${_pri_extract_dir}/resources.pri")
    message(FATAL_ERROR "windows_appsdk: resources.pri not found inside ${_runtime_msix_file}")
  endif()
endif()

# Every executable in this build tree links the bootstrap import lib through maui_core, so the
# app-local Bootstrap.dll must be loadable by ALL of them (test binaries included). They land in the
# binary dir root — stage both files there once at configure time.
file(COPY "${_bootstrap_dll}" DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY "${_pri_extract_dir}/resources.pri" DESTINATION "${CMAKE_BINARY_DIR}")

function(maui_windows_deploy target)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${_bootstrap_dll}" "$<TARGET_FILE_DIR:${target}>/Microsoft.WindowsAppRuntime.Bootstrap.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${_pri_extract_dir}/resources.pri" "$<TARGET_FILE_DIR:${target}>/resources.pri"
    COMMENT "windows_appsdk: deploying Bootstrap.dll + resources.pri beside ${target}")
endfunction()
