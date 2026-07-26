# CMake toolchain: cross-compile Windows x86-64 binaries FROM macOS/Linux with mingw-w64.
#
# Scope — read this before using it. This lane exists to build and test the port's CROSS-PLATFORM core
# (graphics primitives, layout, the property/binding system, the handler mappers) as a real Windows
# binary, and to build the Win32 pipeline smoke app. It is NOT the parity backend:
#
#   MAUI's Windows backend is WinUI 3 (Microsoft.UI.Xaml.Controls.Button, TextBlock — verified in
#   src/Core/src/Handlers/*/(*)Handler.Windows.cs), which requires MSVC + the Windows SDK + the Windows
#   App SDK and CANNOT be built by mingw-w64. Visual comparison against MAUI therefore only ever
#   happens on the MSVC lane (docs/WINDOWS_TOOLCHAIN.md §5), built on the guest.
#
# Using mingw for a *visual* backend would repeat a mistake this project has already paid for: the
# Android SearchBar was implemented with framework compound drawables instead of the real AppCompat
# SearchView, and its parity page is permanently yellow because a stand-in widget cannot match the
# reference's rendering. A Win32/GDI control set would be that mistake applied to an entire platform.
#
# Usage (no CMakePresets entry yet — deliberately: CMakeLists.txt has no `windows` branch in its
# platform-source selection, so a `windows-mingw` preset would configure the port and then fail at the
# first missing backend source. Add the preset together with that branch; until then invoke directly):
#
#   brew install mingw-w64
#   cmake -S . -B build/windows-mingw -G Ninja \
#         -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-mingw.cmake
#   cmake --build build/windows-mingw
#
# Verified against a scratch project (configure → build → PE32+). The Win32 pipeline smoke app does not
# go through CMake at all — see tools/parity/windows/build_smoke.sh.
#
# Override the triple prefix if a different mingw build is in use:
#   -DMAUI_MINGW_PREFIX=x86_64-w64-mingw32

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(MAUI_MINGW_PREFIX "x86_64-w64-mingw32" CACHE STRING "mingw-w64 target triple prefix")

find_program(MAUI_MINGW_C_COMPILER   "${MAUI_MINGW_PREFIX}-gcc")
find_program(MAUI_MINGW_CXX_COMPILER "${MAUI_MINGW_PREFIX}-g++")
find_program(MAUI_MINGW_RC_COMPILER  "${MAUI_MINGW_PREFIX}-windres")

if(NOT MAUI_MINGW_CXX_COMPILER)
  message(FATAL_ERROR
    "mingw-w64 not found (${MAUI_MINGW_PREFIX}-g++).\n"
    "  macOS: brew install mingw-w64\n"
    "  Debian/Ubuntu: apt install mingw-w64")
endif()

set(CMAKE_C_COMPILER   "${MAUI_MINGW_C_COMPILER}")
set(CMAKE_CXX_COMPILER "${MAUI_MINGW_CXX_COMPILER}")
if(MAUI_MINGW_RC_COMPILER)
  set(CMAKE_RC_COMPILER "${MAUI_MINGW_RC_COMPILER}")
endif()

# Search host programs on the host, but headers/libraries ONLY in the target sysroot — otherwise CMake
# happily finds a macOS .dylib and links it into a PE, failing far from the cause.
get_filename_component(_maui_mingw_bin "${MAUI_MINGW_CXX_COMPILER}" DIRECTORY)
get_filename_component(_maui_mingw_root "${_maui_mingw_bin}" DIRECTORY)
set(CMAKE_FIND_ROOT_PATH "${_maui_mingw_root}/${MAUI_MINGW_PREFIX}" "${_maui_mingw_root}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BEFORE)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Self-contained binaries: a stock Windows guest has NO mingw runtime DLLs, and a missing one surfaces
# only as a launch-time modal dialog — which the E2E runner can report no more precisely than "process
# exited early", costing a VM session to diagnose.
#
# `-static` (fully static), NOT just `-static-libstdc++ -static-libgcc`: verified with objdump on a real
# binary from this toolchain, the latter still leaves **libwinpthread-1.dll** as a dynamic import
# (GCC's threading model links it separately). Only the UCRT (api-ms-win-crt-*.dll, present on
# Windows 10+) and the system DLLs should remain — tools/parity/windows/build_smoke.sh gates on exactly
# that and will fail the build if a lib*.dll import reappears.
#
# NOTE for callers: do NOT pass -municode/-mwindows via a command-line -DCMAKE_EXE_LINKER_FLAGS=… — that
# REPLACES this _INIT value (losing -static) and, worse, breaks CMake's compiler ABI check, whose test
# program has a plain main() and cannot link against the wide GUI entry point. Set the GUI/wide-entry
# flags per TARGET instead:
#     target_link_options(<gui_target> PRIVATE -municode -mwindows)
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static")

# Windows.h defines min/max macros that break std::min/std::max, and pulls in a very large surface by
# default. Both defines are what every Win32 C++ codebase sets; doing it here keeps it off every
# translation unit's shoulders.
add_compile_definitions(
  WIN32_LEAN_AND_MEAN
  NOMINMAX
  UNICODE
  _UNICODE            # the port is UTF-16-at-the-boundary on Windows; wWinMain / *W APIs
)
