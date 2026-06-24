# Gallery sample resources

Bundled assets for the runnable demo gallery (the `gallery` target of the standalone `examples/` project).
The gallery otherwise ships only `Info.plist` + the binary, so every `image_source::from_file(...)` /
`from_font(...)` source in `pages/*.hpp` failed to load. These files make those sources render on device.

All files are **faithful copies of the original .NET MAUI sample assets** (read-only `src/` reference or
the local `~/maui-compare` clone) — nothing here is hand-drawn or fabricated.

| File | Bytes | Provenance (read-only original) |
|------|-------|---------------------------------|
| `dotnet_bot.png` | 92532 | `~/maui-compare/Resources/Images/dotnet_bot.png` (the rasterized .NET-bot; `src/` ships only `dotnet_bot.svg`) |
| `animated_heart.gif` | 15462 | `src/Controls/samples/Controls.Sample/Resources/Images/animated_heart.gif` |
| `settings.png` | 1948 | `src/Controls/samples/Controls.Sample/Resources/Images/settings.png` |
| `coffee.png` | 490 | `src/Controls/samples/Controls.Sample/Resources/Images/coffee.png` |
| `oasis.jpg` | 125291 | `src/Controls/samples/Controls.Sample/Resources/Images/oasis.jpg` |
| `cover1.jpg` | 5168 | `src/Controls/tests/TestCases.HostApp/Resources/Images/cover1.jpg` |
| `ionicons.ttf` | 164548 | `src/Controls/samples/Controls.Sample/Resources/Fonts/ionicons.ttf` (family name `Ionicons`; the `Font Image Source` glyph `U+F30C` is present → glyph 527) |

## Still missing (not fabricated)

`cog.png` and `thumb_image.png` exist in the original MAUI sample **only as vector SVGs**
(`src/Controls/samples/Controls.Sample/Resources/Images/{cog,thumb_image}.svg`); MAUI's Resizetizer
rasterizes them to PNGs at build time. The C++ port's image loaders (`[UIImage imageNamed:]` on iOS,
`NSImage initWithContentsOfFile:` on macOS) cannot decode raw SVG, and no SVG rasterizer is available in
this environment, so a faithful raster cannot be produced without fabricating pixels. Pages that reference
`cog.png` (e.g. `image_button`) therefore still show no icon — matching the existing parity note, and the
C# reference shot where the button fill dominates and the cog is barely visible.

## How they are bundled

See `examples/gallery/CMakeLists.txt` (`maui_add_app(gallery ... RESOURCES <these files>)`):
- **iOS** (a flat `.app`): `maui_add_app`'s POST_BUILD step copies every file into the `.app` root, where
  `[UIImage imageNamed:]` finds them; `ionicons.ttf` is registered at launch via `UIAppFonts` in the
  gallery's `gallery_info.plist.in`.
- **macOS / headless** (a plain executable whose loader resolves relative paths against the CWD): `maui_add_app`
  copies the same files next to the binary; run the gallery from a directory where they resolve. The font is
  registered by the framework's font seam. Caveat: *auto-sized* images in a stack (e.g. the Image page's
  `FileSource`, which has no Width/HeightRequest) may collapse to zero height on macOS — the
  `image_handler::get_desired_size` returns `{0,0}` ("no intrinsic content size this cut", unlike the iOS
  handler's `sizeThatFits:`). That intrinsic-measure gap is a pre-existing, deferred concern, independent of
  asset bundling.
