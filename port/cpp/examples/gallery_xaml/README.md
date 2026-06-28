# gallery_xaml — the demo gallery, authored in XAML

This is the **with-XAML twin** of [`examples/gallery`](../gallery): the same 59 demo pages, but each one is
authored as MAUI markup and built at **compile time** with `#embed` + `maui::build_page` — exactly the path
the other `_xaml` examples ([`counter_xaml`](../counter_xaml), [`hello_world_xaml`](../hello_world_xaml))
use. It is the "C++ & XAML" column of the 3-way parity comparison (MAUI | C++ builder | C++ & XAML).

## Layout (MAUI-style)

```
gallery_xaml/
  main.cpp                       app shell: pick one page at runtime, host it
  Views/
    <name>.xaml                  the page UI, as MAUI markup (the moved gallery twins)
    <name>.xaml.hpp              declares examples::Views::<name>_page()  (the .xaml.cs factory analog)
    <name>.xaml.cpp             #embed "<name>.xaml" + build_page<no_view_model, …>()   (the impl)
    gallery_pages.hpp            generated aggregator: all includes + the MAUI_XAML_GALLERY_PAGES X-macro
  ViewModels/                    (MAUI-style; empty — every twin is purely structural, see ViewModels/README.md)
  Models/                        (MAUI-style; empty — see Models/README.md)
  tools/gen_pages.py             stamps the .xaml.hpp/.xaml.cpp wrappers + gallery_pages.hpp from Views/*.xaml
```

Each `Views/<name>.xaml.cpp` is the C++ analog of a MAUI `*.xaml.cs` generated partial: the compiler embeds
the raw markup (`#embed`) and `build_page` hydrates it through the runtime XAML loader into a fresh
`content_page`. Because there is **no external codegen step**, this is the one XAML path that cross-compiles
into the iOS app bundle. The twins are purely structural (any list data is inline `x:Array`), so they use
`maui::no_view_model` — no view-model and no code-behind.

## Adding / changing a page

1. Add or edit `Views/<name>.xaml`.
2. Re-stamp the wrappers + aggregator: `python3 tools/gen_pages.py`.
3. Re-configure (the `Views/*.xaml.cpp` glob is resolved at configure time).

The same `Views/*.xaml` corpus is the one the framework's **load gate**
(`tests/xaml/gallery_twin_tests.cpp`) and **render gate** (`tests/hosting/gallery_twin_render_tests.cpp`)
validate headlessly — they read it via `GALLERY_TWINS_DIR`, pointed here.

## Run

Selects one page with `MAUI_SAMPLE_PAGE` (or the `MAUI_XAML_TWIN` alias); default `value_controls`. Theme
follows the system appearance trait (no per-app env), like the other `_xaml` examples.

```sh
# headless / macOS (built by the standalone examples project):
MAUI_SAMPLE_PAGE=label ./gallery_xaml

# iOS parity capture (its own bundle id, alongside the builder gallery):
xcrun simctl install booted examples/build-ios/gallery_xaml/gallery_xaml.app
xcrun simctl ui booted appearance light
SIMCTL_CHILD_MAUI_SAMPLE_PAGE=label xcrun simctl launch booted dev.maui-cpp.ios-gallery-xaml
```
