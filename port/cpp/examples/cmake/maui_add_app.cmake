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
# The Mac Catalyst variant (macOS bundle, no LSRequiresIPhoneOS — see the file's note).
set(MAUI_EXAMPLE_MACCATALYST_PLIST_IN "${CMAKE_CURRENT_LIST_DIR}/maccatalyst_app_info.plist.in"
    CACHE INTERNAL "Mac Catalyst Info.plist template for example .app bundles")

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
  elseif(MAUI_BACKEND STREQUAL "maccatalyst")
    set(_app_plist "${MAUI_EXAMPLE_MACCATALYST_PLIST_IN}")
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

  if(MAUI_BACKEND STREQUAL "maccatalyst")
    # Mac Catalyst: a macOS .app bundle (Contents/ layout) holding the macabi binary — it launches NATIVELY
    # on macOS as a Catalyst app (the iOS UIKit handlers retargeted to macabi; see the framework's
    # cmake/maccatalyst.toolchain.cmake). MACOSX_BUNDLE on this Darwin build produces Contents/MacOS +
    # Info.plist; the Catalyst plist (default above) carries NO LSRequiresIPhoneOS.
    set_target_properties(${name} PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST "${_app_plist}"
      MACOSX_BUNDLE_BUNDLE_NAME "${name}"
      MACOSX_BUNDLE_GUI_IDENTIFIER "${_app_identifier}"
      MACOSX_BUNDLE_BUNDLE_VERSION "1"
      MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    if(ARG_RESOURCES)
      # Catalyst apps look up bundled assets under Contents/Resources (the macOS bundle convention), unlike
      # the flat iOS .app root — copy them there via a generator-agnostic POST_BUILD step.
      add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:${name}>/Resources"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ARG_RESOURCES}
                "$<TARGET_BUNDLE_CONTENT_DIR:${name}>/Resources"
        COMMENT "Bundling resources into ${name}.app (Catalyst)")
    endif()
  elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
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
  elseif(MAUI_BACKEND STREQUAL "windows")
    # Windows (WinUI 3): a WINDOWED executable, so no console window flashes into every parity capture.
    # /ENTRY:mainCRTStartup is what makes that free: the WINDOWS subsystem normally implies a WinMain
    # entry point, and this keeps the plain main() that maui/maui_main.hpp defines — so an example's
    # source stays byte-identical across backends, which is the whole contract of this helper.
    # (WIN32_EXECUTABLE TRUE alone would set the subsystem AND the entry point, and the link would then
    # fail looking for WinMain.)
    target_link_options(${name} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
    # The Windows App Runtime bootstrap DLL MUST sit beside the exe. Without it the process dies at load
    # with 0xC0000135 BEFORE main() runs: no dialog, no log line, and the E2E runner sees only "the
    # process exited early" — which reads as an app bug rather than a missing file.
    if(NOT MAUI_WASDK_BOOTSTRAP_DLL)
      message(FATAL_ERROR "MAUI_WASDK_BOOTSTRAP_DLL is not set; configure the framework for "
                          "MAUI_BACKEND=windows through tools/parity/windows/configure_port_windows.ps1")
    endif()
    add_custom_command(TARGET ${name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MAUI_WASDK_BOOTSTRAP_DLL}" "$<TARGET_FILE_DIR:${name}>"
      COMMENT "Copying Microsoft.WindowsAppRuntime.Bootstrap.dll next to ${name}")
    # Win2D's native DLL, beside the exe for the same reason the bootstrap DLL above is -- but with a much
    # quieter failure mode. C++/WinRT activates Microsoft.Graphics.Canvas.* by probing this DLL on the
    # default search path (see the framework CMakeLists.txt windows block for the base.h line numbers), so
    # if it is missing nothing crashes: font_image_source_service::load catches the activation failure and
    # falls back to a blank transparent glyph, i.e. the page renders exactly as it did before Win2D existed.
    # That is a parity regression that looks like "the fix didn't work", so fail the CONFIGURE instead.
    if(NOT MAUI_WIN2D_DLL)
      message(FATAL_ERROR "MAUI_WIN2D_DLL is not set; configure the framework for "
                          "MAUI_BACKEND=windows through tools/parity/windows/configure_port_windows.ps1")
    endif()
    add_custom_command(TARGET ${name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MAUI_WIN2D_DLL}" "$<TARGET_FILE_DIR:${name}>"
      COMMENT "Copying Microsoft.Graphics.Canvas.dll (Win2D) next to ${name}")
    # WebView2's two native DLLs, beside the exe for the same reason Win2D's is and with the same quiet
    # failure mode: C++/WinRT activates Microsoft.Web.WebView2.Core.* by probing Microsoft.Web.WebView2.
    # Core.dll on the default search path, and that DLL in turn loads WebView2Loader.dll from the same
    # directory to reach the installed Evergreen runtime. Miss either and EnsureCoreWebView2Async fails at
    # activation: web_view/hybrid_web_view render nothing and the board looks exactly as it did before the
    # handlers existed. Fail the CONFIGURE instead. (See the framework CMakeLists.txt WebView2 block for
    # why neither DLL comes from the WindowsAppRuntime framework package.)
    # NOTE: WebView2 creates its user-data folder as <exe>.WebView2 NEXT TO THE EXE, so the deploy
    # directory has to be WRITABLE -- a new environmental dependency the port did not previously have.
    if(NOT MAUI_WEBVIEW2_CORE_DLL OR NOT MAUI_WEBVIEW2_LOADER_DLL)
      message(FATAL_ERROR "MAUI_WEBVIEW2_CORE_DLL / MAUI_WEBVIEW2_LOADER_DLL are not set; configure the "
                          "framework for MAUI_BACKEND=windows through "
                          "tools/parity/windows/configure_port_windows.ps1")
    endif()
    add_custom_command(TARGET ${name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${MAUI_WEBVIEW2_CORE_DLL}" "${MAUI_WEBVIEW2_LOADER_DLL}" "$<TARGET_FILE_DIR:${name}>"
      COMMENT "Copying the WebView2 runtime DLLs next to ${name}")
    if(ARG_RESOURCES)
      add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ARG_RESOURCES}
                "$<TARGET_FILE_DIR:${name}>"
        COMMENT "Copying resources next to ${name}")
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
