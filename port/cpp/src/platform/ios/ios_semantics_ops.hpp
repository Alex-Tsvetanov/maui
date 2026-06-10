#pragma once
// Shared UIKit operations for the generic-IView accessibility metadata (Semantics) and the
// InputTransparent flag — the platform side of the shared view_mapper's map_semantics /
// map_input_transparent (view_mapper.cpp), and the iOS twin of apple_semantics_ops.hpp. Objective-C++
// only — include exclusively from .mm files compiled as Objective-C++ (it references UIView / the
// UIAccessibility properties).
//
// Unlike the AppKit twin (which ADAPTED these to NSAccessibility + a -hitTest: gate), the C# originals
// ARE the iOS implementations, so the helpers below port them directly:
//
// apply_semantics ports Microsoft.Maui.Platform.SemanticExtensions.UpdateSemantics(UIView, Semantics)
// (src/Core/src/Platform/iOS/SemanticExtensions.cs):
//   - a null Semantics is a no-op (C#: `if (semantics == null) return;` — the properties are left as-is);
//   - Description maps to UIView.accessibilityLabel and Hint to UIView.accessibilityHint. The port's
//     semantics carries non-null std::strings whose unset value is "" (C#'s unset is null), so an empty
//     string maps to nil — preserving UIKit's built-in fallbacks (e.g. a UIButton announcing its title)
//     exactly as a null C# Description does;
//   - when a Description or Hint is present (not null/whitespace) the view is marked an accessibility
//     element so they are surfaced — except UIControl-derived views, which already are elements (with
//     C#'s carve-out for UIStepper / UIPageControl, composite controls iOS marks NOT elements);
//   - IsHeading adds UIAccessibilityTraitHeader to accessibilityTraits when missing, and removes it when
//     no longer heading ONLY if it was present — exactly C#'s hasHeader keying.
//   (The C# UISearchBar branch — redirecting to the inner search text field — has no analog: the port
//   has no search-bar control yet.)
//
// apply_input_transparent ports ViewExtensions.UpdateInputTransparent (iOS,
// src/Core/src/Platform/iOS/ViewExtensions.cs): UIView.userInteractionEnabled = !InputTransparent, so a
// transparent view is excluded from hit-testing (its touches pass through to whatever is behind it) —
// UIKit has the flag natively, unlike AppKit. C#'s ITextInput overload (folding IsReadOnly into the
// flag) belongs to the text controls' own mappers, not this generic seam, so only the general branch is
// ported here. Accessibility is intentionally untouched: UserInteractionEnabled does not affect
// VoiceOver, and Semantics/InputTransparent are independent mappers (independent C# extensions).

#import <UIKit/UIKit.h>

#include <algorithm>
#include <cctype>
#include <string>

#include "maui/core/semantics.hpp"

namespace maui::platform::ios
{
    namespace detail
    {
        // C# string.IsNullOrWhiteSpace over the port's non-null std::string: empty or all-whitespace.
        // (The "null" half collapses onto empty — the port's unset string IS "".)
        inline bool is_whitespace_only(const std::string& value)
        {
            return std::ranges::all_of(value, [](unsigned char character) { return std::isspace(character) != 0; });
        }

        // Make a std::string into an NSString for the accessibility setters, mapping EMPTY to nil (the
        // port's "" is C#'s null Description/Hint — nil keeps UIKit's own fallback label). The _Nullable
        // stringWithUTF8String: (nil on invalid UTF-8) needs no extra guard: the setters accept nil.
        inline NSString* to_accessibility_string(const std::string& value)
        {
            if (value.empty())
            {
                return nil;
            }
            return [NSString stringWithUTF8String:value.c_str()];
        }
    } // namespace detail

    // Push a Semantics borrow (null = leave as-is) onto the native view's accessibility properties.
    inline void apply_semantics(UIView* view, const maui::core::semantics* value)
    {
        if (view == nil || value == nullptr)
        {
            return; // C#: a null Semantics leaves the platform accessibility properties untouched
        }
        const std::string& description = value->description();
        const std::string& hint = value->hint();
        view.accessibilityLabel = detail::to_accessibility_string(description);
        view.accessibilityHint = detail::to_accessibility_string(hint);

        if (!detail::is_whitespace_only(hint) || !detail::is_whitespace_only(description))
        {
            // Most UIControl elements automatically have IsAccessibilityElement set to true; UIStepper and
            // UIPageControl inherit from UIControl but iOS marks them NOT elements (composite controls), so
            // C# re-marks exactly those two. (C# spells this as two identically-bodied branches; folded into
            // one condition here — same truth table.)
            const bool is_control = [view isKindOfClass:[UIControl class]];
            const bool is_composite_control =
                [view isKindOfClass:[UIStepper class]] || [view isKindOfClass:[UIPageControl class]];
            if (!is_control || is_composite_control)
            {
                view.isAccessibilityElement = YES;
            }
        }

        const UIAccessibilityTraits accessibility_traits = view.accessibilityTraits;
        const bool has_header = (accessibility_traits & UIAccessibilityTraitHeader) == UIAccessibilityTraitHeader;
        if (value->is_heading())
        {
            if (!has_header)
            {
                view.accessibilityTraits = accessibility_traits | UIAccessibilityTraitHeader;
            }
        }
        else
        {
            if (has_header)
            {
                view.accessibilityTraits = accessibility_traits & ~UIAccessibilityTraitHeader;
            }
        }
    }

    // Push the InputTransparent flag onto the native view: UserInteractionEnabled = !InputTransparent
    // (ViewExtensions.UpdateInputTransparent's general branch), excluding the view from hit-testing.
    inline void apply_input_transparent(UIView* view, bool value)
    {
        if (view == nil)
        {
            return;
        }
        view.userInteractionEnabled = static_cast<BOOL>(!value);
    }
} // namespace maui::platform::ios
