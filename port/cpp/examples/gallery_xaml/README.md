# gallery_xaml — the demo gallery, authored in XAML

This is the **with-XAML twin** of [`examples/gallery`](../gallery): the same demo pages, but each one is
authored as MAUI markup and built at **compile time** with `#embed` + `maui::build_page` — exactly the path
the other `_xaml` examples ([`counter_xaml`](../counter_xaml), [`hello_world_xaml`](../hello_world_xaml))
use. It is the "C++ & XAML" column of the 3-way parity comparison (MAUI | C++ builder | C++ & XAML).

## Layout (MAUI-style)

```
gallery_xaml/
  main.cpp                       app shell: pick one page at runtime, host it
  Views/
    <name>.xaml                  LEGACY local twins pending migration to port/maui-reference/pages/
    <name>.xaml.hpp              declares examples::Views::<name>_page()  (the .xaml.cs factory analog)
    <name>.xaml.cpp             #embed the markup + build_page<no_view_model, …>()   (the impl)
    gallery_pages.hpp            generated aggregator: all includes + the MAUI_XAML_GALLERY_PAGES X-macro
  ViewModels/                    (MAUI-style; empty — every twin is purely structural, see ViewModels/README.md)
  Models/                        (MAUI-style; empty — see Models/README.md)
```

The `.xaml.hpp`/`.xaml.cpp` wrappers + `gallery_pages.hpp` are stamped by the **unified E2E tool**
(`python3 port/tools/e2e/e2e.py gen` — superseded the old in-tree `tools/gen_pages.py`). Page markup is
authored as **canonical shared pages** in `port/maui-reference/pages/` — the exact same `.xaml` bytes
real .NET MAUI compiles (XamlC + code-behind) and this app `#embed`s, per
`port/maui-reference/docs/AUTHORING.md`. The remaining `Views/*.xaml` files are legacy local twins that
have not migrated yet; a shared page with the same key supersedes its legacy twin.

Each generated `Views/<name>.xaml.cpp` is the C++ analog of a MAUI `*.xaml.cs` generated partial: the
compiler embeds the raw markup (`#embed`) and `build_page` hydrates it through the runtime XAML loader
into a fresh `content_page`. Because there is **no external codegen step**, this is the one XAML path
that cross-compiles into the iOS app bundle. The twins are purely structural (any list data is inline
`x:Array`), so they use `maui::no_view_model` — no view-model and no code-behind.

## Adding / changing a page

1. Author/edit the canonical shared markup: `port/maui-reference/pages/<name>.xaml` (see
   `port/maui-reference/docs/AUTHORING.md` — naming triple, no event attributes, manifest row).
   (Legacy-only edit: `Views/<name>.xaml` still works until that page migrates.)
2. Re-stamp the wrappers + aggregator: `python3 port/tools/e2e/e2e.py gen`.
3. Re-configure (the `Views/*.xaml.cpp` glob is resolved at configure time).

The twin corpus (shared pages + legacy `Views/*.xaml`) is what the framework's **load gate**
(`tests/xaml/gallery_twin_tests.cpp`) and **render gate** (`tests/hosting/gallery_twin_render_tests.cpp`)
validate headlessly — they read it via `GALLERY_TWINS_DIR`.

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
