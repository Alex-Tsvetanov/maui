# gallery — the runnable demo gallery, as a pure-C++ example (no .mm). main.cpp selects ONE of the curated
# demo pages (gallery/pages/*.hpp) at runtime from MAUI_SAMPLE_PAGE and the theme from MAUI_APPEARANCE; the
# framework's native run_app supplies the run loop + forces the native interface style from the app theme.
#
# Two non-default knobs vs the curated examples (both via maui_add_app options):
#   IDENTIFIER — keep the bundle id dev.maui-cpp.ios-gallery so the iOS parity tooling's simctl launch keeps
#                working unchanged (tools/parity/capture_all.py).
#   PLIST      — gallery_info.plist.in adds UIAppFonts (ionicons.ttf) so the bundled icon font registers at
#                launch (the Image page's "Font Image Source" rows need the Ionicons family; the port has no
#                runtime font-registration seam).
#
# The demo assets are bundled flat (next to the binary on headless/macOS, into the .app root on iOS) so the
# pages' from_file("dotnet_bot.png") / imageNamed: loads resolve — the same flat layout the framework's old
# in-tree gallery used.

set(GALLERY_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/animated_heart.gif
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/coffee.png
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/cover1.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/dotnet_bot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/ionicons.ttf
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/oasis.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/settings.png)

maui_add_app(gallery
  SOURCES main.cpp
  RESOURCES ${GALLERY_RESOURCES}
  IDENTIFIER dev.maui-cpp.ios-gallery
  PLIST ${CMAKE_CURRENT_SOURCE_DIR}/gallery_info.plist.in)
