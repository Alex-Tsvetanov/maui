# C++ port vs .NET MAUI — visual parity comparison (Mac Catalyst)

Side-by-side of representative pages rendered by **real, shipped .NET MAUI on Mac Catalyst** (left) and by the **C++ port** on macOS/AppKit (right). The C# side is a standalone `dotnet new maui` app (`~/maui-compare`, shipped MAUI workload) reproducing the same pages; both are captured via each framework's own self-screenshot. C# Mac Catalyst is the direct analog to the C++ macOS/AppKit backend (both "MAUI on Mac", both top-left origin).

> Toolchain notes: the machine has Xcode 26.5; the .NET-for-iOS pack was updated to the Xcode-26.5 build (`26.5.10284`) and built with `-p:ValidateXcodeVersion=false`. Mac Catalyst is used (native, no simulator). The C# captures use a dark system appearance (Mac Catalyst default); the C++ gallery uses a light window — compare **layout/geometry**, not chrome theme.

| Page | .NET MAUI (Mac Catalyst) | C++ port (macOS) |
| --- | --- | --- |
| Control stack | ![cs](csharp_maccatalyst/controls_stack.png) | ![cpp](../examples/value_controls/macos.png) |
| Layout alignment (Start/Center/End/Fill) | ![cs](csharp_maccatalyst/alignment.png) | ![cpp](../examples/border_alignment/macos.png) |
| Shapes | ![cs](csharp_maccatalyst/shapes.png) | ![cpp](../examples/polygon_gallery/macos.png) |
| Border (RoundRectangle stroke) | ![cs](csharp_maccatalyst/border.png) | ![cpp](../examples/border_styles/macos.png) |
| CollectionView (grid, templated cells) | ![cs](csharp_maccatalyst/collectionview.png) | ![cpp](../examples/selection_mode/macos.png) |
| Fonts (sizes + attributes) | ![cs](csharp_maccatalyst/fonts.png) | ![cpp](../examples/fonts/macos.png) |
| Grid (rows × cols) | ![cs](csharp_maccatalyst/grid.png) | ![cpp](../examples/grid/macos.png) |
| Gradient brushes | ![cs](csharp_maccatalyst/gradient.png) | ![cpp](../examples/brushes/macos.png) |

## What this validated

Running the real MAUI side surfaced/confirmed the macOS rendering bug you spotted: the C++ AppKit backend rendered **bottom-up** (unflipped NSView origin) while MAUI renders **top-down**. The `alignment` reference (Start at top → Fill at bottom, each block aligned left/center/right) is the clearest check — it confirms both the **AppKit-flip fix** (top-down) and the **View.HorizontalOptions fix** (per-block alignment). The C++ macOS captures here update to the corrected top-down render once the flip fix lands.
