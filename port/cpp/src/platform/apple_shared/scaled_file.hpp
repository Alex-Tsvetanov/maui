#pragma once
// Shared Apple @2x/@3x scaled-file probe — used by BOTH the AppKit (apple) and UIKit (ios) file image
// services. Pure std::filesystem (no AppKit/UIKit/CoreFoundation), so this header is includable from any
// Apple-backend TU and is deterministically unit-testable; the caller supplies the screen scale.
//
// Ports Microsoft.Maui.ImageSourceExtensions.GetScaledFile (ImageSourceExtensions.iOS.cs). MAUI ships
// this only in its iOS/MacCatalyst partial, but macOS asset catalogs use the same @Nx convention, so the
// AppKit file service reuses it for parity (the apple twin previously loaded the original file directly).
//
// ios_image_ops.hpp re-exports scaled_file / get_scaled_file into maui::platform::ios for the existing iOS
// call sites + tests; the canonical definition lives here in maui::platform::apple_shared.

#include <algorithm>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <system_error>

namespace maui::platform::apple_shared
{
    // The result of the @2x/@3x probe: the file to decode plus the scale its pixels are authored at
    // (C# GetScaledFile's return + `out int scale`).
    struct scaled_file
    {
        std::string path;
        int scale = 1;
    };

    // Port of ImageSourceExtensions.GetScaledFile: when the screen scale is > 1, probe
    // "<name>@<s>x<ext>" beside `filename` from min(3, screen_scale) down to 2 and return the first
    // sibling that exists (with its scale); otherwise — or when nothing scaled exists — the original
    // filename at scale 1. The screen scale is a PARAMETER (the .mm reads UIScreen/NSScreen) so the probe
    // itself stays deterministic for tests.
    inline scaled_file get_scaled_file(std::string_view filename, int screen_scale)
    {
        constexpr int max_scale = 3; // C# MaxScale: "max of 3 seems to be what Apple has gone up to"
        constexpr int min_scale = 2; // C# MinScale: "only 2 because 1 is 'no scale'"
        if (screen_scale > 1)
        {
            const std::filesystem::path original{std::string(filename)};
            const std::filesystem::path dir = original.parent_path();
            const std::string name = original.stem().string();
            const std::string ext = original.extension().string();
            for (int s = std::min(max_scale, screen_scale); s >= min_scale; --s)
            {
                const std::filesystem::path scaled = dir / std::format("{}@{}x{}", name, s, ext);
                std::error_code ec;
                if (std::filesystem::exists(scaled, ec))
                {
                    return {.path = scaled.string(), .scale = s};
                }
            }
        }
        return {.path = std::string(filename), .scale = 1};
    }
} // namespace maui::platform::apple_shared
