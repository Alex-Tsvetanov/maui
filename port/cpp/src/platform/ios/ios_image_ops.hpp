#pragma once
// Shared iOS image-pipeline helpers — used by the UIKit image services (image_source_services.mm) and
// their on-simulator tests. Pure C++ + CoreFoundation only (no UIKit), so the header is includable from
// any iOS-backend TU; the sibling of ios_text_ops.hpp / ios_conversions.hpp.
//
//   - get_scaled_file  <-  Microsoft.Maui.ImageSourceExtensions.GetScaledFile (iOS): the @2x/@3x
//     sibling-file probe for the current screen scale. Pure std::filesystem (deterministically
//     unit-testable; the caller supplies the screen scale).
//   - cf_ref           — a minimal RAII owner for a +1 CoreFoundation reference (CGImageSource /
//     CGImage / CGImageDestination), CFRelease'd on scope exit (PROFILE §8: RAII at the native
//     boundary; ARC does not manage CF types).

#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <system_error>

namespace maui::platform::ios
{
    // Owns one +1 CoreFoundation reference (or null), released on destruction. Scope-bound only
    // (non-copyable, non-movable) — the decode helpers never hand ownership onward.
    template <class T> class cf_ref
    {
    public:
        explicit cf_ref(T ref) noexcept : ref_(ref)
        {
        }
        cf_ref(const cf_ref&) = delete;
        cf_ref& operator=(const cf_ref&) = delete;
        cf_ref(cf_ref&&) = delete;
        cf_ref& operator=(cf_ref&&) = delete;
        ~cf_ref()
        {
            if (ref_ != nullptr)
            {
                CFRelease(ref_);
            }
        }

        [[nodiscard]] T get() const noexcept
        {
            return ref_;
        }
        explicit operator bool() const noexcept
        {
            return ref_ != nullptr;
        }

    private:
        T ref_;
    };

    // The result of the @2x/@3x probe: the file to decode plus the scale its pixels are authored at
    // (C# GetScaledFile's return + `out int scale`).
    struct scaled_file
    {
        std::string path;
        int scale = 1;
    };

    // Port of ImageSourceExtensions.GetScaledFile (ImageSourceExtensions.iOS.cs): when the screen scale
    // is > 1, probe "<name>@<s>x<ext>" beside `filename` from min(3, screen_scale) down to 2 and return
    // the first sibling that exists (with its scale); otherwise — or when nothing scaled exists — the
    // original filename at scale 1. The screen scale is a PARAMETER (the .mm reads UIScreen) so the
    // probe itself stays deterministic for tests.
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
} // namespace maui::platform::ios
