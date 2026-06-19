# Gallery sample resources

Bundled assets for the runnable demo galleries (`maui_ios_gallery` / `maui_macos_gallery`). The galleries
otherwise ship only `Info.plist` + the binary, so every `image_source::from_file(...)` / `from_font(...)`
source in `src/samples/pages/*.hpp` failed to load. These files make those sources render on device.

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

See `port/cpp/CMakeLists.txt` (the gallery targets):
- **iOS** (`maui_ios_gallery`, a flat `.app`): a POST_BUILD step copies every file into the `.app` root, where
  `[UIImage imageNamed:]` finds them; `ionicons.ttf` is registered at launch via `UIAppFonts` in the
  gallery's `ios_gallery_info.plist.in`.
- **macOS** (`maui_macos_gallery`, a plain executable whose loader resolves relative paths against the CWD):
  the same files are copied next to the binary; `macos_gallery.mm` `chdir`s there at startup and registers
  the font at runtime via `CTFontManagerRegisterFontsForURL`. Verified: pages with **explicitly-sized**
  images render the bundled assets (e.g. the `image_button` "Custom Size" button shows the purple
  `dotnet_bot.png`). Caveat: *auto-sized* images in a stack (e.g. the Image page's `FileSource`, which has
  no Width/HeightRequest) still collapse to zero height — the macOS `image_handler::get_desired_size`
  returns `{0,0}` ("no intrinsic content size this cut", unlike the iOS handler's `sizeThatFits:`). That
  intrinsic-measure gap is a pre-existing, deliberately-deferred concern, independent of asset bundling.
