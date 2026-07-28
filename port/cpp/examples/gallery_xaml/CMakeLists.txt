# gallery_xaml — the WITH-XAML twin of examples/gallery, via COMPILE-TIME XAML (#embed + maui::build_page),
# structured like a MAUI app: main.cpp (app shell) + Views/<name>.xaml{,.hpp,.cpp} per page (markup +
# page-factory declaration + #embed/build_page implementation). No codegen tool, so it builds on every
# backend including the iOS bundle. main.cpp selects ONE page at runtime from MAUI_SAMPLE_PAGE (or the
# MAUI_XAML_TWIN alias); theme follows the system appearance trait — same knobs the parity tooling drives.
#
# Two non-default maui_add_app options match the builder gallery so the iOS parity capture is unchanged:
#   IDENTIFIER — its own stable bundle id (dev.maui-cpp.ios-gallery-xaml) so simctl install/launch can host
#                the builder gallery and this XAML gallery side by side.
#   PLIST      — gallery_xaml_info.plist.in adds UIAppFonts (ionicons.ttf) so the image twin's
#                FontImageSource rows resolve the Ionicons family (the port has no runtime font-registration).
#
# The demo assets are SHARED with the builder gallery (examples/gallery/resources/) — bundled flat next to
# the binary (headless/macOS) or into the .app root (iOS) so the twins' from_file()/imageNamed: loads resolve.

set(GALLERY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../gallery)
set(GALLERY_XAML_RESOURCES
    ${GALLERY_DIR}/resources/animated_heart.gif
    ${GALLERY_DIR}/resources/coffee.png
    ${GALLERY_DIR}/resources/cog.png
    ${GALLERY_DIR}/resources/cog@2x.png
    ${GALLERY_DIR}/resources/cover1.jpg
    ${GALLERY_DIR}/resources/dotnet_bot.png
    ${GALLERY_DIR}/resources/ionicons.ttf
    ${GALLERY_DIR}/resources/oasis.jpg
    ${GALLERY_DIR}/resources/photo.jpg
    ${GALLERY_DIR}/resources/settings.png)

# One translation unit per page (Views/<name>.xaml.cpp, each owning its #embed). Configure-time glob: a new
# twin (added via tools/gen_pages.py) needs a re-configure to be picked up — same convention as the gallery.
#
# ...EXCEPT where the compiler has no #embed. It is a C23/C++26 feature: AppleClang implements it (so the
# iOS/macOS/headless lanes use the committed TUs verbatim), MSVC does not, and neither does the NDK's
# Clang 18. For those, generate a bytes-mode twin of every page at configure time and build THOSE — the
# markup becomes an `unsigned char[]` literal fed into the SAME fixed_string/build_page path, so the
# rendered result is identical and the hand-written code-behind is carried over unchanged. This is the
# same escape hatch and the same generator the android app-host already uses (see the
# maui_android_apphost_xaml block in port/cpp/CMakeLists.txt); it is factored no further because the two
# call sites want different outputs (a .so's source list vs this executable's).
#
# Generated into the BUILD tree, never into Views/, so the committed #embed TUs stay untouched.
if(MSVC)
  set(GALLERY_XAML_BYTES_DIR ${CMAKE_CURRENT_BINARY_DIR}/Views_bytes)
  # port/cpp/examples/gallery_xaml -> port/tools/e2e/e2e.py
  set(MAUI_E2E_TOOL ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/e2e/e2e.py)
  if(NOT EXISTS ${MAUI_E2E_TOOL})
    message(FATAL_ERROR
      "gallery_xaml needs bytes-mode codegen on this compiler (no #embed) but the generator is missing: "
      "${MAUI_E2E_TOOL}. It lives in the port tree, so this path only resolves for an in-tree build "
      "(-DMAUI_EXAMPLES_FRAMEWORK_DIR=...).")
  endif()
  find_program(MAUI_PYTHON3 NAMES python3 python REQUIRED)
  execute_process(
    COMMAND ${MAUI_PYTHON3} ${MAUI_E2E_TOOL}
            gen --embed-mode=bytes --out-dir ${GALLERY_XAML_BYTES_DIR}
    RESULT_VARIABLE gen_rc OUTPUT_VARIABLE gen_out ERROR_VARIABLE gen_err)
  if(NOT gen_rc EQUAL 0)
    message(FATAL_ERROR "e2e.py gen --embed-mode=bytes failed (rc=${gen_rc}):\n${gen_err}")
  endif()
  message(STATUS "gallery_xaml bytes-mode codegen: ${gen_out}")
  file(GLOB GALLERY_XAML_PAGE_SOURCES ${GALLERY_XAML_BYTES_DIR}/*.xaml.cpp)
else()
  file(GLOB GALLERY_XAML_PAGE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Views/*.xaml.cpp)
endif()

maui_add_app(gallery_xaml
  SOURCES main.cpp ${GALLERY_XAML_PAGE_SOURCES}
  RESOURCES ${GALLERY_XAML_RESOURCES}
  IDENTIFIER dev.maui-cpp.ios-gallery-xaml
  PLIST ${CMAKE_CURRENT_SOURCE_DIR}/gallery_xaml_info.plist.in)

# The example root is on the include path so the Views/... includes (gallery_pages.hpp and each .xaml.hpp)
# resolve. #embed of the sibling .xaml uses the source file's own directory, so it needs no extra path.
target_include_directories(gallery_xaml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# build_page hydrates the embedded markup through the runtime loader -> link the XAML layer.
target_link_libraries(gallery_xaml PRIVATE maui::xaml)
# The committed .xaml.cpp TUs use #embed (a C23/C++26 feature clang accepts under -std=c++23 but warns
# on). Not applicable to the bytes-mode lane above, which is plain C++23 — and -Wno-c23-extensions is a
# clang spelling MSVC would reject outright.
if(NOT MSVC)
  target_compile_options(gallery_xaml PRIVATE -Wno-c23-extensions)
endif()
