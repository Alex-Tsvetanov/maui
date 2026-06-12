#pragma once
// Shared AppKit operations for the generic-IView accessibility metadata
// (Semantics) and the InputTransparent flag — the platform side of the shared
// view_mapper's map_semantics / map_input_transparent (view_mapper.cpp).
// Objective-C++ only — include exclusively from .mm files compiled as
// Objective-C++ (it references NSView / the NSAccessibility properties / the
// objc runtime).
//
// apply_semantics ports
// Microsoft.Maui.Platform.SemanticExtensions.UpdateSemantics (iOS,
// src/Core/src/Platform/iOS/SemanticExtensions.cs) to AppKit:
//   - a null Semantics is a no-op (C#: `if (semantics == null) return;` — the
//   property is left as-is);
//   - Description maps to NSView.accessibilityLabel (iOS AccessibilityLabel —
//   the accessible name);
//   - Hint        maps to NSView.accessibilityHelp  (iOS AccessibilityHint —
//   the extra usage detail);
//   - when a Description or Hint is present the view is marked an accessibility
//   element
//     (NSView.accessibilityElement = YES), mirroring C# setting
//     IsAccessibilityElement = true so the label/help are actually surfaced
//     (AppKit controls already are elements; setting it is idempotent);
//   - IsHeading toggles the heading role: C# adds/removes
//   UIAccessibilityTrait.Header; the AppKit analog
//     is NSView.accessibilityRole = NSAccessibilityHeadingRole (set when
//     heading, cleared — restored to the role AppKit would otherwise report,
//     via accessibilityRole = nil — when not), plus the "heading"
//     accessibilityRoleDescription so VoiceOver announces it as a heading. The
//     clear path keys on whether the view currently carries the heading role,
//     exactly as C# keys on the existing trait.
//
// apply_input_transparent ports ViewExtensions.UpdateInputTransparent (iOS):
// iOS sets UIView.UserInteractionEnabled = !InputTransparent so the view is
// excluded from hit-testing (its touches pass through to whatever is behind
// it). AppKit has no per-view UserInteractionEnabled and — unlike UIKit — a
// container's -hitTest: does NOT recurse through its subviews' -hitTest:, so
// toggling a property is not enough: the view's OWN -hitTest: must return nil.
// We therefore install (once per view class, idempotently) a -hitTest: gate
// that returns nil when a per-instance "input transparent" associated flag is
// set, and otherwise defers to the original implementation — the faithful
// AppKit equivalent of UserInteractionEnabled = false (and the same mechanism
// MAUI's own LayoutView.HitTest uses to drop an InputTransparent layout from
// hit-testing). Setting the flag false restores normal hit-testing.
// Accessibility is intentionally untouched here: UserInteractionEnabled does
// not affect VoiceOver on iOS, and Semantics/InputTransparent are independent
// mappers (independent C# extensions).

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <string>

#include "maui/core/semantics.hpp"

namespace maui::platform::apple
{
    namespace detail
    {
        // The associated-object key whose presence on an NSView marks it
        // input-transparent (value @YES; absent = opaque). A unique address is the
        // canonical objc-runtime key idiom.
        inline const char k_input_transparent_key = 0;
        // Marks a class as already carrying the installed -hitTest: gate (so we install
        // at most once per class, never re-wrapping our own gate).
        inline const char k_hit_test_gate_installed_key = 0;

        // Make a std::string into an NSString, guarding the _Nullable
        // stringWithUTF8String: (nil on invalid UTF-8) so the accessibility setters
        // (which want non-null) always receive a string.
        inline NSString* to_ns_string(const std::string& value)
        {
            NSString* const raw = [NSString stringWithUTF8String:value.c_str()];
            return raw != nil ? raw : @"";
        }

        // Install, once per class, a -hitTest: that returns nil when the per-instance
        // input-transparent flag is set and otherwise calls the previous
        // implementation. The previous IMP is captured and invoked through a typed
        // function pointer (the standard way to chain a replaced objc method).
        inline void install_hit_test_gate(Class cls)
        {
            if (objc_getAssociatedObject(cls, &k_hit_test_gate_installed_key) != nil)
            {
                return; // already gated this class — never wrap our own gate
            }
            const SEL selector = @selector(hitTest:);
            Method method = class_getInstanceMethod(cls, selector);
            if (method == nullptr)
            {
                return; // no -hitTest: in the hierarchy (cannot happen for NSView subclasses) — nothing to gate
            }
            const IMP original = method_getImplementation(method);
            // The replacement: gate on the per-instance flag, else defer to the original
            // IMP. The IMP is a plain C function pointer; invoking it needs its true
            // (self, _cmd, point) signature.
            using hit_test_fn = NSView* (*)(NSView*, SEL, NSPoint);
            const auto original_fn = reinterpret_cast<hit_test_fn>(original);
            IMP gate = imp_implementationWithBlock(^NSView*(NSView* view, NSPoint point) {
              if (objc_getAssociatedObject(view, &k_input_transparent_key) != nil)
              {
                  return nil; // input-transparent: pass the hit through
                              // (UserInteractionEnabled = false)
              }
              return original_fn(view, selector, point);
            });
            // Prefer replacing on THIS class so sibling instances of other classes are
            // untouched; if the class only inherited -hitTest: (did not define its own),
            // class_replaceMethod adds it here.
            class_replaceMethod(cls, selector, gate, method_getTypeEncoding(method));
            objc_setAssociatedObject(cls, &k_hit_test_gate_installed_key, @YES, OBJC_ASSOCIATION_RETAIN);
        }
    } // namespace detail

    // Push a Semantics borrow (null = leave as-is) onto the native view's
    // accessibility properties.
    inline void apply_semantics(NSView* view, const maui::core::semantics* value)
    {
        if (view == nil || value == nullptr)
        {
            return; // C#: a null Semantics leaves the platform accessibility properties
                    // untouched
        }
        const std::string& description = value->description();
        const std::string& hint = value->hint();
        view.accessibilityLabel = detail::to_ns_string(description);
        view.accessibilityHelp = detail::to_ns_string(hint);

        // C# marks the view an accessibility element when it carries a Description or
        // Hint so they are surfaced (UIControl-derived views already are; setting it
        // is harmless and idempotent).
        if (!description.empty() || !hint.empty())
        {
            view.accessibilityElement = YES;
        }

        if (value->is_heading())
        {
            view.accessibilityRole = NSAccessibilityHeadingRole;
            view.accessibilityRoleDescription = NSAccessibilityRoleDescription(NSAccessibilityHeadingRole, nil);
        }
        else if ([view.accessibilityRole isEqualToString:NSAccessibilityHeadingRole])
        {
            // Was a heading, no longer is: restore the role AppKit would otherwise
            // report (nil = default) — the analog of C# clearing the Header trait only
            // when it was present.
            view.accessibilityRole = nil;
            view.accessibilityRoleDescription = nil;
        }
    }

    // Push the InputTransparent flag onto the native view: when true the view is
    // excluded from hit-testing (its own -hitTest: returns nil), the AppKit analog
    // of UserInteractionEnabled = false.
    inline void apply_input_transparent(NSView* view, bool value)
    {
        if (view == nil)
        {
            return;
        }
        detail::install_hit_test_gate([view class]);
        // Presence of the flag (== @YES) means transparent; clearing it (nil)
        // restores hit-testing.
        objc_setAssociatedObject(view, &detail::k_input_transparent_key, value ? @YES : nil,
                                 OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
} // namespace maui::platform::apple
