# maui_add_app — the ONE platform-aware helper for the standalone examples project.
#
# Every example's own CMakeLists stays trivial and 100% portable:
#
#     maui_add_app(counter SOURCES main.cpp)
#
# This function owns ALL per-backend packaging so the example SOURCE never names a platform:
#   - headless / macOS (apple): a plain executable. On headless it boots, mounts, settles one layout
#     pass and exits 0 (no run loop); on apple it opens a real NSWindow and enters [NSApp run].
#   - iOS: an installable .app bundle with a generated Info.plist (a launchable simulator app needs the
#     bundle + plist; the plain console layout the framework's own test exes use is not launchable).
#
# The bundle + plist approach is factored from how the in-tree framework builds maui_ios_app_sample /
# maui_ios_gallery (the add_executable + MACOSX_BUNDLE + MACOSX_BUNDLE_INFO_PLIST block in
# port/cpp/CMakeLists.txt): set MACOSX_BUNDLE TRUE and point MACOSX_BUNDLE_INFO_PLIST at a template the
# CMake bundle generator configures with the per-target name/id/version vars below.
#
# The framework target maui::hosting carries C++23, the public include path, and the whole link DAG
# (controls -> core -> graphics, plus the backend's run_app run loop), so an example only links it.
#
# Signature:
#   maui_add_app(<name> SOURCES <src>... [RESOURCES <file>...] [IDENTIFIER <bundle-id>] [PLIST <plist.in>])
#     SOURCES    — the example's pure-C++ translation units (required; one defines main via maui_main.hpp).
#     RESOURCES  — optional asset files copied next to the binary (macOS/headless) or into the .app bundle
#                  root (iOS), the same flat layout the framework's gallery POST_BUILD step uses.
#     IDENTIFIER — optional iOS bundle id override (default dev.maui-cpp.examples.<name>). The gallery sets
#                  it to dev.maui-cpp.ios-gallery so the parity tooling's simctl launch keeps working.
#     PLIST      — optional Info.plist.in template override (default examples/cmake/ios_app_info.plist.in).
#                  The gallery uses a variant that adds UIAppFonts so its bundled ionicons.ttf registers.

# The Info.plist template lives beside this file (examples/cmake/). Resolved once, at include time, to an
# absolute path so it is correct no matter which example subdirectory calls the function.
set(MAUI_EXAMPLE_IOS_PLIST_IN "${CMAKE_CURRENT_LIST_DIR}/ios_app_info.plist.in"
    CACHE INTERNAL "iOS Info.plist template for example .app bundles")

function(maui_add_app name)
  cmake_parse_arguments(ARG "" "IDENTIFIER;PLIST" "SOURCES;RESOURCES" ${ARGN})
  if(NOT ARG_SOURCES)
    message(FATAL_ERROR "maui_add_app(${name}): SOURCES is required")
  endif()
  # Per-target bundle id + Info.plist template, each with a sensible default (the curated examples pass
  # neither; the gallery overrides both to keep its bundle id stable + register its bundled font).
  if(ARG_IDENTIFIER)
    set(_app_identifier "${ARG_IDENTIFIER}")
  else()
    set(_app_identifier "dev.maui-cpp.examples.${name}")
  endif()
  if(ARG_PLIST)
    set(_app_plist "${ARG_PLIST}")
  else()
    set(_app_plist "${MAUI_EXAMPLE_IOS_PLIST_IN}")
  endif()

  add_executable(${name} ${ARG_SOURCES})
  # maui::hosting is the framework's top-of-DAG target (PUBLIC C++23 + include dir + the whole link
  # graph + the backend run loop). Linking it is the ONLY framework dependency an example declares.
  target_link_libraries(${name} PRIVATE maui::hosting)
  # Belt-and-braces: maui::hosting already PUBLIC-requires cxx_std_23, but pin it on the example too so
  # an example builds at C++23 even if consumed against a future relaxed framework requirement.
  target_compile_features(${name} PRIVATE cxx_std_23)

  if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # iOS: an installable .app bundle. Opt into MACOSX_BUNDLE with a generated Info.plist (the same
    # minimal non-scene plist the framework's iOS samples use — simulator apps need no code signing).
    set_target_properties(${name} PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST "${_app_plist}"
      MACOSX_BUNDLE_BUNDLE_NAME "${name}"
      MACOSX_BUNDLE_GUI_IDENTIFIER "${_app_identifier}"
      MACOSX_BUNDLE_BUNDLE_VERSION "1"
      MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    if(ARG_RESOURCES)
      # iOS bundles are flat — copy assets to the .app content root (where [UIImage imageNamed:] and
      # UIAppFonts look), via a generator-agnostic POST_BUILD copy (the framework gallery's approach;
      # the RESOURCE target property would nest them under Resources/, which imageNamed: would miss).
      add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ARG_RESOURCES}
                "$<TARGET_BUNDLE_CONTENT_DIR:${name}>"
        COMMENT "Bundling resources into ${name}.app")
    endif()
  else()
    # headless / apple: a plain executable. Copy any resources next to the binary (the loader resolves
    # from_file() paths against the CWD — same convention as the framework's plain macOS gallery).
    if(ARG_RESOURCES)
      add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ARG_RESOURCES}
                "$<TARGET_FILE_DIR:${name}>"
        COMMENT "Copying resources next to ${name}")
    endif()
  endif()
endfunction()
