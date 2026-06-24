# =====================================================================================================
# maui_examples — a STANDALONE CMake project demonstrating idiomatic, OS-agnostic use of the C++23
# .NET MAUI port. It is NOT part of the framework's own build: it consumes the framework as an external
# package and stands entirely on its own.
#
# Two consume modes (pick one):
#
#   (A) Installed package — find_package against an installed framework prefix:
#         # build + install the framework once (from port/cpp):
#         cmake --preset headless && cmake --build --preset headless
#         cmake --install build/headless --prefix /tmp/maui-prefix
#         # then configure THIS project against it (note the vcpkg toolchain for the pugixml dep):
#         cmake -S examples -B examples/build \
#               -DCMAKE_PREFIX_PATH=/tmp/maui-prefix \
#               -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
#         cmake --build examples/build
#       This is the default path below: find_package(maui CONFIG REQUIRED).
#
#   (B) In-tree subdirectory — build against the framework sources without installing, by pointing
#       MAUI_EXAMPLES_FRAMEWORK_DIR at the framework root (port/cpp). Configure with:
#         cmake -S examples -B examples/build -DMAUI_EXAMPLES_FRAMEWORK_DIR=..
#       This add_subdirectory()s the framework, which provides the same maui::* ALIAS targets
#       find_package would, so the example CMake is identical either way.
#
# Either way the framework's pugixml transitive dependency must resolve (the vcpkg toolchain handles it
# in mode A via find_dependency; mode B's add_subdirectory pulls it through the framework's own manifest).
# =====================================================================================================
cmake_minimum_required(VERSION 3.28)
project(maui_examples CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Acquire the framework: in-tree add_subdirectory OR installed find_package ----
if(DEFINED MAUI_EXAMPLES_FRAMEWORK_DIR)
  # Mode (B): consume the framework sources directly. Its CMakeLists defines the maui::* ALIASes (and its
  # vcpkg manifest brings pugixml), so nothing else here changes. A binary dir is required for an
  # out-of-source add_subdirectory of a sibling/parent path.
  message(STATUS "maui_examples: consuming framework in-tree from ${MAUI_EXAMPLES_FRAMEWORK_DIR}")
  add_subdirectory(${MAUI_EXAMPLES_FRAMEWORK_DIR} ${CMAKE_BINARY_DIR}/_maui_framework)
else()
  # Mode (A): consume an installed framework via its exported package config (mauiConfig.cmake), which
  # re-finds pugixml and pulls in the maui:: namespaced imported targets.
  find_package(maui CONFIG REQUIRED)
  message(STATUS "maui_examples: consuming installed framework (find_package maui)")
endif()

# ---- The shared per-platform packaging helper (the ONLY platform-aware CMake in this project) ----
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(maui_add_app)

# ---- The examples (each its own subfolder: a pure-C++ main.cpp + a trivial CMakeLists) ----
# Ordered simplest-first, the order the README walks a newcomer through.
add_subdirectory(hello_world)     # minimal app: window -> page -> label
add_subdirectory(counter)         # button::clicked increments a label
add_subdirectory(layouts)         # vertical_stack_layout + grid composition
add_subdirectory(data_binding)    # a bindable_object view-model bound to a label, driven by an entry
add_subdirectory(collection_view) # collection_view over an observable_collection + a data_template
add_subdirectory(custom_drawing)  # graphics_view + an i_drawable that draws shapes and text

# The runnable demo gallery: one app hosting whichever of the curated demo pages (gallery/pages/*.hpp) the
# MAUI_SAMPLE_PAGE env var selects. The iOS parity-capture pipeline builds + launches THIS target (bundle id
# dev.maui-cpp.ios-gallery) — see tools/parity/capture_all.py.
add_subdirectory(gallery)         # runtime page-select demo gallery (MAUI_SAMPLE_PAGE / MAUI_APPEARANCE)
