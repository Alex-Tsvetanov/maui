# Parity review — open items needing a user ruling

Per `port/CLAUDE.md` ruling 3: a MAUI-side quirk not covered by rulings 1–10 is recorded here with its
evidence and **paused for a ruling** — neither auto-ignored nor auto-fixed. Approved rulings get appended
to the list in `port/CLAUDE.md`.

---

## 1. ✅ RESOLVED (2026-07-16) — `src/` vs SHIPPED MAUI CheckBox sizing: the RENDER wins

**Ruling (user, 2026-07-16): the render wins — drop the 44 floor.** Now `port/CLAUDE.md` ruling 11.
Implemented: `check_box_handler.mm` `minimumViewSize = k_default_size` (18pt, not 44). Verified on the
Catalyst board — `entry` +20px → **0 / 0.00**, `border_playground` → **0 / 0.37**; the checkbox row is now
18pt, its content bands identical to MAUI (150/183/209/219). Headless 3765/3765; iOS 10/2331 (== baseline,
no regression). The original analysis is kept below for the record.

### The measurement

`entry` is a **+20px pure offset** — but only for the block `y=160..460`; above and below it, the port
matches MAUI exactly (residual **0.00**). The divergence starts at one element: the page's
`<CheckBox IsChecked="True" />` (`port/maui-reference/pages/entry.xaml:9`).

Measured geometry, from the captures (spacing calibrated from the entry pitch, not assumed):

| | checkbox row |
| --- | --- |
| MAUI **Mac Catalyst** — gap between the bracketing Entry borders `150 → 183` = 33px, minus 2× spacing (9.5px) | **14px ≈ 18pt** |
| MAUI **iOS** (1:1, no Catalyst scale) — `216.7 → 259.0` = 42.3pt, minus 2× spacing (12.3pt) | **17.7pt ≈ 18pt** |
| **the port** (Catalyst) — `150 → 203` = 53px, minus 2× spacing | **34px ≈ 44pt** |

44 − 18 = 26pt = **20.02px** at the 0.77 Catalyst UIKit scale — exactly the measured offset. The glyph
itself is identical (14px) in both; only the row differs. MAUI renders 18pt on **both** platforms, so this
is **not** a Mac Catalyst "renders-less" quirk (ruling 10).

### The conflict

- **`src/` (the documented behavior oracle) says 44.** `CheckBoxHandler.iOS.cs:8` `protected virtual float
  MinimumSize => 44f`, applied via `CreatePlatformView`'s `MinimumViewSize = MinimumSize`, and
  `MauiCheckBox.SizeThatFits` (`:245-253`) floors both axes at it:
  `final = Min(Max(MinimumViewSize, h), Max(MinimumViewSize, w))` ⇒ ≥ 44. iOS's
  `ViewHandlerExtensions.iOS.GetDesiredSizeFromHandler` does call `SizeThatFits`, so this path is live.
  There is no override of `MinimumSize` anywhere in `src/` (Compatibility excluded), and the
  Controls-level `CheckBox.Mapper.cs` only remaps `Color` — it does not touch the measure.
- **The shipped MAUI renders 18** = `MauiCheckBox.DefaultSize` (`:13` `const float DefaultSize = 18.0f`),
  i.e. the natural size with **no floor applied**.

**Why they differ:** they are different MAUI versions. `src/`'s `MauiCheckBox.cs` was last modified
**2025-01-26** (`2b0d66669c`), while `port/maui-reference/app/MauiReference.csproj:12` pins
**`<MauiVersion>10.0.71</MauiVersion>`** — a much newer NuGet. The port is written against `src/`; the
ground-truth captures are rendered by 10.0.71.

The port already ports the `src/` rule faithfully (`check_box_handler.mm:245-253` mirrors
`SizeThatFits`; `:340` sets `minimumViewSize = 44.0`), which is exactly why it renders 44.

### The fork (why this is not mine to pick)

The two authorities point opposite ways and this case is not covered by rulings 1–10:

- **`port/CLAUDE.md` prime directive** — `src/` is the behavior oracle; "no invented behavior… don't ship
  a guess". Following it ⇒ keep 44, accept `entry` staying red vs the reference.
- **Ruling 1** — "Microsoft's MAUI render IS the ground truth for all page CONTENT… any content
  difference vs MAUI is a port bug to fix". Following it ⇒ drop the floor so the port renders 18.

Ruling 1 was written about MAUI's *rendered demos* vs the port, not about `src/` contradicting the shipped
binary, so applying it here would extend it. Picking either silently would be a guess about a MAUI version
I cannot read.

### What I need

A ruling on the general question, since this will recur wherever the 2025 `src/` snapshot and the 10.0.71
NuGet have diverged:

1. **When `src/` and the shipped `MauiVersion` disagree, which wins?** (My recommendation: **the render**,
   per ruling 1 — the board's whole purpose is matching what MAUI actually draws; `src/` is then the
   oracle only for *mechanism*, not for values it has since changed.) If so, I'd drop the 44 floor with a
   comment recording that `src/` says otherwise and why.
2. Or: **bump `src/`** to the 10.0.71 tag so the oracle and the reference agree (larger, but removes the
   whole class of conflict).
3. Or: **pin the reference app to a MAUI matching `src/`** (~2025) so the captures match the oracle.

Until then `entry` is 🟡 blocked, not 🔴. Everything else on the page matches at residual 0.00.

**Evidence to reproduce:**
```
python3 port/cpp/tools/parity/capture_ios_clean.py --app maui --themes light --only entry   # MAUI iOS
# then measure the Entry borders bracketing the CheckBox in a text-free column.
```

---

## 2. 🟡 `nested_collection` cpp_xaml blank — XAML loader can't realize a nested templated CollectionView

**Status:** open — larger framework work, not a ruling. The last genuinely-open macOS red
(`nested_collection`, 3.64% — mostly white). NOT a port rendering bug: the port's own framework renders
it correctly (see below); the gap is specific to the **compile-time-XAML loader**.

### The finding

`nested_collection.xaml` is an OUTER `CollectionView` (VerticalList) whose ItemTemplate root is an INNER
`CollectionView` (HorizontalList, HeightRequest=100) with its own `Header="{Binding Title}"`,
HeaderTemplate, `ItemsSource="{Binding Items}"`, and ItemTemplate (`{Binding Caption}`). The outer
ItemsSource is assigned in **code-behind** (NestedCollectionPage.xaml.cs), not in the XAML.

Measured (clean captures): `maui` 72 content rows, `cpp` (code-first) 102 content rows — both render
fully; **`xaml` 0 content rows — completely blank** (only the banner).

### Two-part cause — part 1 fixed locally, part 2 is the blocker

- **Part 1 (data — SOLVED, but reverted with part 2).** cpp_xaml used `build_page<no_view_model, …>` so
  the outer CV had NO source. I built the `super_teams`-style bindable VM (`ViewModels/nested_sources.hpp`:
  `gallery_item` with a registered `Caption`, `nested_source` with `Title` + `Items`) and wired
  `nested_collection.xaml.cpp` (`find<collection_view>("CollectionView")` → `set_items_source`). VERIFIED
  by diagnostic: `find` succeeds, **20 items set, `item_template` + `items_layout` both present**.
- **Part 2 (rendering — THE BLOCKER).** With all of the above set, the outer CV STILL realizes **zero
  cells**. `basic_grouping` uses the identical late-`set_items_source` pattern and renders, so it is not a
  data/timing issue — the XAML loader cannot **realize an outer cell whose DataTemplate root is a nested
  CollectionView** (with its own Header/ItemTemplate + `{Binding Items}`). The code-first `cpp` column
  builds that template in C++ (`data_template::of<collection_view>()` + `add_setup` for the inner
  ItemsLayout, which "is a plain slot, NOT a bindable_property, so it cannot be staged through the
  template's Values" — nested_collection_page.hpp) and works; the XAML template-inflater path does not.

The part-1 VM was reverted because alone it produces NO board change (still blank) — shipping it would be
correct-but-dead code implying progress. It is ~80 lines following `super_teams.hpp` and trivially
recreated alongside the loader work.

### What it needs

XAML-loader support for realizing a DataTemplate whose root is a templated `CollectionView` — staging the
inner CV's non-bindable `ItemsLayout`, its `{Binding Items}`/`{Binding Title}` against each outer item's
binding context, and its own Header/ItemTemplate. This is a real layer-6 feature, not a page tweak; the
port's framework (cpp) already renders the page, so it is a demo-column-only gap at low pixel impact.

**Evidence to reproduce:** wire the VM as above, add `fprintf(stderr, …)` after `set_items_source`, launch
`gallery_xaml` with `MAUI_SAMPLE_PAGE=nested_collection` and stderr redirected — shows 20 items +
item_template set, render still blank.
