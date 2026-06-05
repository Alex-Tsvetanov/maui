# STATUS — MAUI → C++23 port progress

> One row per component. Update at step 7 of the per-component loop (`CLAUDE.md`). Never mark a
> partial port as done silently — use the Notes column.
> Legend: ✅ done · 🚧 in progress · ⬜ not started · — n/a

## Milestones (see `PROJECT.md §5`)

| Milestone | Description | Status |
|---|---|---|
| M0 | Graphics primitives compile + pass ported tests | ⬜ |
| M1 | Core contracts + property/handler infra + dispatcher, unit-tested | ⬜ |
| M2 | `button` end-to-end (headless → macOS), tap works in sample app | ⬜ |
| M3 | Layout measure/arrange (`stack_layout`, `grid`) pass layout tests | ⬜ |
| M4 | Control set v1 (label, entry, image, layouts, page) on macOS | ⬜ |
| M5 | `bindable_object`/`bindable_property`, binding, style, lifecycle | ⬜ |
| M6 | Second platform (iOS) behind the same handlers | ⬜ |
| M7 | XAML and/or Essentials (as prioritized) | ⬜ |

## Components

| Component | Layer | Contract✓ | Tests ported | Impl | Platform(s) | Notes |
|---|---|---|---|---|---|---|
| project scaffold (CMake/presets/vcpkg/Catch2) | infra | — | — | ⬜ | headless | green smoke test = scaffold done |
| `color` | graphics | ⬜ | ⬜ | ⬜ | — | from `src/Graphics/src/Graphics/Color.cs` |
| `point_f`/`size_f`/`rect_f` | graphics | ⬜ | ⬜ | ⬜ | — | |
| geometry / `path_f` | graphics | ⬜ | ⬜ | ⬜ | — | |
| `event<>` + `dispatcher` | core | ⬜ | ⬜ | ⬜ | — | infra |
| `bindable_object`/`bindable_property` | core | ⬜ | ⬜ | ⬜ | — | value precedence; verify vs Core.UnitTests |
| `i_element`/`i_view`/`i_text` | core | ⬜ | ⬜ | ⬜ | — | virtual-view contracts |
| `view_handler` base + handler registry | core | ⬜ | ⬜ | ⬜ | — | CRTP + `i_view_handler` |
| `button` (handler slice) | controls/handlers | ⬜ | ⬜ | ⬜ | headless→macOS | the Rosetta Stone (M2) |

_(extend this table as components are added)_
