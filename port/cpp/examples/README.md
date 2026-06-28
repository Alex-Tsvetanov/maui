# MAUI C++ examples

A standalone CMake project of small, single-concept programs that show idiomatic, OS-agnostic use of the
C++23 .NET MAUI port. Every example is **pure portable C++** — no Objective-C, no `.mm`, no platform
headers — written in the MAUI `MauiProgram` style: an `application` subclass plus the one
`use_shared_maui_app` configurator. The same source builds and runs on the headless, macOS, and iOS
backends; only the build helper (`cmake/maui_add_app.cmake`) is platform-aware.

## The examples

- **hello_world** — the minimal app: a window hosting a page hosting a single label.
- **counter** — the classic interactive example: a button's `clicked` event increments a label.
- **layouts** — composing UI with a `vertical_stack_layout` and a 2x2 `grid` (rows/columns, spacing, padding).
- **data_binding** — a `bindable_object` view-model bound to a label by name, driven live by an `entry`.
- **collection_view** — a list built from an `observable_collection` and a `data_template` cell recipe.
- **custom_drawing** — a `graphics_view` rendering an `i_drawable` that paints shapes and text.
- **gallery** — the full runnable demo gallery (~178 pages from `gallery/pages/`). It selects ONE page at
  runtime from the `MAUI_SAMPLE_PAGE` env var (default `value_controls`) and the theme from `MAUI_APPEARANCE`
  (`dark`|`light`), still 100% pure C++ — the framework's iOS/macOS `run_app` forces the native interface
  style from the app theme. Its iOS bundle id is `dev.maui-cpp.ios-gallery` (the parity tooling launches it).
  Run a page: `MAUI_SAMPLE_PAGE=pickers ./examples/build/gallery/gallery` (macOS/headless), or on the sim
  `SIMCTL_CHILD_MAUI_SAMPLE_PAGE=pickers xcrun simctl launch booted dev.maui-cpp.ios-gallery`.

### With-XAML twins (`*_xaml`)

`hello_world`, `counter`, `data_binding`, and `layouts` each have a `*_xaml` twin that authors the **same**
UI as raw XAML markup instead of a hand-written builder, via **compile-time XAML** (`#embed` +
`maui::build_page`): the compiler embeds the `.xaml` bytes into the binary — no codegen tool, no build step,
so it cross-compiles to the iOS bundle. Each twin is structured like a MAUI `.xaml`/`.xaml.cs` app, as three
linked files:

- **`<page>.xaml`** — the markup.
- **`<page>.xaml.hpp`** — a bodyless declaration of the page-construction function in `namespace examples`,
  e.g. `std::unique_ptr<maui::controls::content_page> counter_page(std::unique_ptr<counter_view_model>);`
  (bound pages take the view-model, like the design doc's `LoginPage(std::unique_ptr<LoginViewModel>)`;
  structural pages take no argument).
- **`<page>.xaml.cpp`** — a **separate translation unit** implementing it: the `#embed`, `build_page`,
  `bind_to`, and any `x:Name` code-behind wiring live here (the view-model type is sealed in this TU).
  Code-behind event tokens are parked on the page with `page->retain(...)` so the function can return the
  page by value. The view-model, when present, is in its own header (`<vm>.hpp`).

`main.cpp` is then just the app shell — it `#include`s the `.xaml.hpp`, constructs the view-model, calls the
factory, and hosts the page; it never sees `#embed`/`build_page`. CMake links both TUs:
`maui_add_app(<name> SOURCES main.cpp <page>.xaml.cpp)`. The twins render pixel-identically to the builder
examples on device — see [docs/comparison/EXAMPLES_XAML.md](../docs/comparison/EXAMPLES_XAML.md).

## Building

The examples consume the framework as an external package. There are two modes.

### Mode A — against an installed framework (`find_package`)

First build and install the framework once (commands are relative to `port/cpp`):

```sh
export VCPKG_ROOT="$HOME/vcpkg"
cmake --preset headless
cmake --build --preset headless
cmake --install build/headless --prefix /tmp/maui-prefix
```

Then configure and build this project against that prefix. The framework's one transitive link
dependency is **pugixml**, so the configure step needs the vcpkg toolchain to resolve it via the
package's `find_dependency`; this project's `vcpkg.json` declares pugixml so the toolchain installs it:

```sh
cmake -S examples -B examples/build \
      -DCMAKE_PREFIX_PATH=/tmp/maui-prefix \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build examples/build
```

If `find_package(maui)` does not locate the install under the vcpkg toolchain (which scopes its search
roots), also pass `-Dmaui_DIR=/tmp/maui-prefix/lib/cmake/maui`.

Each example binary boots, mounts its tree, settles one layout pass, and exits 0 on headless. Run e.g.:

```sh
./examples/build/counter/counter
```

### Mode B — against the in-tree framework sources (`add_subdirectory`)

To build without installing, point the project at the framework root (`port/cpp`); it
`add_subdirectory()`s the framework, which provides the same `maui::*` targets `find_package` would:

```sh
cmake -S examples -B examples/build -DMAUI_EXAMPLES_FRAMEWORK_DIR=.. \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build examples/build
```

### iOS

For iOS, build + install the framework against the `ios` preset to a prefix, then configure this project
with the same iOS cache settings the framework's `ios` preset uses (`CMAKE_SYSTEM_NAME=iOS`,
`CMAKE_OSX_SYSROOT=iphonesimulator`, the `arm64-ios-simulator` triplet + overlay):

```sh
cmake --preset ios && cmake --build --preset ios
cmake --install build/ios --prefix /tmp/maui-prefix-ios
cmake -S examples -B examples/build-ios -G Ninja \
      -Dmaui_DIR=/tmp/maui-prefix-ios/lib/cmake/maui \
      -DCMAKE_PREFIX_PATH=/tmp/maui-prefix-ios \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
      -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator \
      -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=26.0 \
      -DVCPKG_TARGET_TRIPLET=arm64-ios-simulator \
      -DVCPKG_OVERLAY_TRIPLETS=$(pwd)/cmake/triplets
cmake --build examples/build-ios --target hello_world
```

`maui_add_app` produces an installable `.app` bundle per example. Install and launch one on a booted
simulator:

```sh
xcrun simctl install booted examples/build-ios/hello_world/hello_world.app
xcrun simctl launch booted dev.maui-cpp.examples.hello_world
```

## How `maui_add_app` works

The helper `maui_add_app(<name> SOURCES <...> [RESOURCES <...>])` creates the executable, links
`maui::hosting` (which carries C++23, the public include path, and the whole framework link DAG including
the backend's run loop), and handles per-platform packaging: a plain executable on headless and macOS; on
iOS an installable `.app` with a generated `Info.plist`. The example sources stay 100% portable — only
this helper names a platform.
