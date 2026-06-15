#pragma once
// Shared iOS image-pipeline helpers — used by the UIKit image services (image_source_services.mm) and
// their on-simulator tests. Pure C++ + CoreFoundation only (no UIKit), so the header is includable from
// any iOS-backend TU; the sibling of ios_text_ops.hpp / ios_conversions.hpp.
//
//   - get_scaled_file  <-  Microsoft.Maui.ImageSourceExtensions.GetScaledFile: the @2x/@3x sibling-file
//     probe for the current screen scale. The pure-std::filesystem implementation now lives in the
//     backend-neutral apple_shared/scaled_file.hpp (shared with the AppKit file service); this header
//     re-exports scaled_file / get_scaled_file into maui::platform::ios for the existing call sites + tests.
//   - cf_ref           — a minimal RAII owner for a +1 CoreFoundation reference (CGImageSource /
//     CGImage / CGImageDestination), CFRelease'd on scope exit (PROFILE §8: RAII at the native
//     boundary; ARC does not manage CF types).

#include <CoreFoundation/CoreFoundation.h>

#include "scaled_file.hpp" // maui::platform::apple_shared::{scaled_file, get_scaled_file}

namespace maui::platform::ios
{
    // Re-export the shared @2x/@3x probe into maui::platform::ios for the existing iOS call sites + tests.
    using maui::platform::apple_shared::get_scaled_file;
    using maui::platform::apple_shared::scaled_file;

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
} // namespace maui::platform::ios
