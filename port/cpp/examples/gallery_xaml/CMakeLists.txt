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
    ${GALLERY_DIR}/resources/settings.png)

# One translation unit per page (Views/<name>.xaml.cpp, each owning its #embed). Configure-time glob: a new
# twin (added via tools/gen_pages.py) needs a re-configure to be picked up — same convention as the gallery.
file(GLOB GALLERY_XAML_PAGE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Views/*.xaml.cpp)

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
# The .xaml.cpp TUs use #embed (a C23/C++26 feature clang accepts under -std=c++23 but warns on).
target_compile_options(gallery_xaml PRIVATE -Wno-c23-extensions)
