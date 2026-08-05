#pragma once
// Shared UIKit operations for the generic-IView flow direction — the platform side of the shared
// view_mapper's map_flow_direction (view_mapper.cpp), and the iOS twin of apple_view_ops.hpp's
// apply_flow_direction. Objective-C++ only — include exclusively from .mm files compiled as
// Objective-C++ (it references UIView / UISemanticContentAttribute).
//
// apply_flow_direction ports Microsoft.Maui.Platform.ViewExtensions.UpdateFlowDirection
// (src/Core/src/Platform/iOS/ViewExtensions.cs:119-148): it maps the resolved FlowDirection to a
// UISemanticContentAttribute (LeftToRight → ForceLeftToRight, RightToLeft → ForceRightToLeft,
// MatchParent → Unspecified — the C# case where the parent has no explicit direction), sets the view's
// semanticContentAttribute, then re-applies the same attribute to each internal subview. The subview
// walk is the iOS-26 workaround StepperExtensions / ProgressBarExtensions both run (the controls
// stopped propagating the attribute to their own subviews); the port's deployment floor IS iOS 26, so
// the walk runs unconditionally (a harmless no-op on OSes that still propagate). The AppKit twin maps
// the same enum to NSView.userInterfaceLayoutDirection instead.
//
// NOTE: this is the SEMANTIC-ATTRIBUTE-ONLY base part. The iOS-26 RTL CGAffineTransform visual flip
// (StepperExtensions.cs:47-74) stays DEFERRED — progress_bar does not apply it either, and this helper
// keeps that consistency. Resolving MatchParent against the parent IView's FlowDirection is the
// handler's job (resolved_flow_direction); this helper applies an already-resolved direction.

#import <UIKit/UIKit.h>

#include "maui/core/flow_direction.hpp"

namespace maui::platform::ios
{
    // ViewExtensions.UpdateFlowDirection's enum → UISemanticContentAttribute mapping (the C# switch:
    // Force{Left,Right}ToLeft; an unresolved MatchParent stays Unspecified — the parent-has-no-explicit-
    // direction case).
    inline UISemanticContentAttribute to_semantic_content_attribute(maui::core::flow_direction fd)
    {
        switch (fd)
        {
            case maui::core::flow_direction::left_to_right:
                return UISemanticContentAttributeForceLeftToRight;
            case maui::core::flow_direction::right_to_left:
                return UISemanticContentAttributeForceRightToLeft;
            case maui::core::flow_direction::match_parent:
                return UISemanticContentAttributeUnspecified;
        }
        return UISemanticContentAttributeUnspecified;
    }

    // Set the (already-resolved) flow direction on the native view as a UISemanticContentAttribute and
    // re-apply it to each internal subview (the iOS-26 subview walk). Mirrors the AppKit twin
    // apply_flow_direction, but with UISemanticContentAttribute instead of NSUserInterfaceLayoutDirection.
    inline void apply_flow_direction(void* native, maui::core::flow_direction fd)
    {
        if (native == nullptr)
        {
            return;
        }
        UIView* const view = (__bridge UIView*)native;
        const UISemanticContentAttribute attribute = to_semantic_content_attribute(fd);
        view.semanticContentAttribute = attribute;
        for (UIView* subview in view.subviews)
        {
            subview.semanticContentAttribute = attribute;
        }
    }

    // ---- lifetime -------------------------------------------------------------------------------
    // Re-fetch a trampoline's virtual view AFTER it has run user code (any send_*/set_* raises an
    // event, and a handler is free to destroy the view it was raised on). `handler` is the raw
    // back-pointer the ObjC trampoline holds; ~<x>_platform's detach_trampolines nulls it, so a null
    // result means "the view died under us" and the caller must stop touching it. NEVER cache the
    // result across another raise — call this again. Pin the trampoline itself with a strong local
    // (`auto* const keep = self;`) before the first raise, or `keep.handler` is itself a freed read.
    // Residual, unfixable here: a view destroyed while a SECOND shared_ptr keeps its handler alive
    // leaves virtual_view_ dangling — nothing in ~view disconnects the handler. App code is safe (the
    // view owns the only handler ref); closing it fully needs maui::controls::view.
    template <class Handler> [[nodiscard]] auto* live_view(Handler* handler) noexcept
    {
        return handler != nullptr ? handler->virtual_view() : nullptr;
    }
} // namespace maui::platform::ios
