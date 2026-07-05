# Authoring rules for the canonical shared XAML pages (`port/maui-reference/pages/`)

Every page in `pages/` is **one file consumed by two frameworks**: real .NET MAUI compiles it (XamlC +
the code-behind partial in `app/Pages/`), and the C++ port `#embed`s the exact same bytes into its
`gallery_xaml` app (`port/cpp/examples/gallery_xaml/Views/<key>.xaml.cpp`, generated). Any rule below
that is violated breaks one side or the other; `e2e.py lint` enforces the mechanical ones.

## The naming triple (lint + static_assert enforced)

| Piece | Form | Example |
| --- | --- | --- |
| page key | `snake_case` filename stem | `activity_indicator` |
| shared markup | `pages/<key>.xaml` | `pages/activity_indicator.xaml` |
| MAUI partial | `x:Class="MauiReference.Pages.<Pascal>Page"` | `MauiReference.Pages.ActivityIndicatorPage` |
| C++ factory | `examples::Views::<key>_page()` (generated) | `activity_indicator_page()` |

Pascal derivation is mechanical: split on `_`, TitleCase each word, append `Page`.

## Skeleton

```xml
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             x:Class="MauiReference.Pages.ActivityIndicatorPage"
             Title="Activity Indicator">
    <VerticalStackLayout Spacing="12" Padding="16">
        <!-- content -->
    </VerticalStackLayout>
</ContentPage>
```

## Rules

1. **Root element is `ContentPage`** with the standard MAUI namespace and the **winfx/2009** `x:`
   namespace (never 2006 — the port parses both, MAUI tooling expects 2009; standardize).
2. **`x:Class` is mandatory** and must match the naming triple (the MAUI build requires it; the port
   recognizes and ignores it; the generated C++ TU `static_assert`s its presence).
3. **NO event attributes** (`Clicked=`, `Tapped=`, `TextChanged=`, `SelectionChanged=`, …). The port
   hydrates a static tree; events in markup would diverge the two frameworks. Interactivity is wired in
   the MAUI code-behind via `x:Name` fields (`MyButton.Clicked += …`), and on the C++ side via
   `page_impl::find<T>("name")`. `e2e.py lint` regex-rejects the known event-attribute names.
4. **Data inline via `x:Array`** (the existing twin convention) — no view-models in tier-1 pages, no
   external data sources; pages must render deterministically for pixel comparison.
5. **Tier-1 pages** (the gallery conversions, `<key>.xaml`) must stay within the port's supported XAML
   feature set — currently avoid: standalone `Trigger`/`DataTrigger`/`EventTrigger` elements,
   `Converter=`/`ConverterParameter=`, `x:TypeArguments` in markup extensions, `x:Arguments`/
   `x:FactoryMethod`. (Supported and encouraged: StaticResource/DynamicResource, Style/Setter,
   DataTemplate/ControlTemplate, AppThemeBinding, OnPlatform/OnIdiom, Binding, x:Array, x:Reference,
   VisualStateGroups, FormattedText/Spans.)
6. **Tier-2 gap pages** (`gap_<feature>.xaml`) do the OPPOSITE deliberately: each uses exactly one
   unsupported feature or unregistered control to pin the port's gap. They MUST build and render in
   MAUI (that's the acceptance gate) and carry a `pages/manifest.json` row whose `expected_port_status`
   records the observed port failure. A gap page that starts loading on the port turns the matching
   test red with "gap closed — update the manifest".
7. **Every page has a `manifest.json` row**:
   ```json
   {
     "key": "activity_indicator",
     "expected_port_status": "renders",   // renders | loads | parse_error | unregistered_type
     "builder_twin": true,                 // has a C++ builder page for structure-equivalence tests
     "tags": ["activity_indicator"],       // controls/features exercised (feeds CONTROL_CATALOG.md)
     "notes": ""
   }
   ```
8. **Determinism**: no wall-clock dates/times, no network resources, no randomness, no animation
   frames that a static capture would race (mirror the gallery's existing conventions).
9. **Native-default rendering**: pages must not depend on the app-level default styles — the reference
   app deliberately does NOT merge `Styles.xaml` (port/CLAUDE.md parity ruling 4). Page-local styles and
   resources are fine (they travel inside the shared markup, so both frameworks apply them equally).

## Adding a page (checklist)

1. Write `pages/<key>.xaml` per the skeleton + rules.
2. Add its `manifest.json` row.
3. `python3 port/tools/e2e/e2e.py gen` — stamps the C++ TU pair + the MAUI code-behind partial
   (replace the generated partial with a hand-written one — drop the GENERATED marker line — if the
   page needs interactivity).
4. `python3 port/tools/e2e/e2e.py lint` — must be clean.
5. Build gates: MAUI app (`dotnet build` per platform) + port twin tests (`ctest -R gallery_twin`).
6. If migrating an existing legacy twin: delete `port/cpp/examples/gallery_xaml/Views/<key>.xaml`
   in the same change (the shared page supersedes it in generation and key derivation).
