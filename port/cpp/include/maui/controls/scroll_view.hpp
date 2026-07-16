#pragma once
// maui::controls::scroll_view  <=  Microsoft.Maui.Controls.ScrollView
//
// A view that scrolls its single content child. Ported from src/Controls/src/Core/ScrollView/
// ScrollView.cs (+ the IScrollViewController surface): Content + Padding, Orientation (default
// Vertical), the two scroll-bar visibilities (default Default), the READ-ONLY ScrollX/ScrollY
// position pair, the read-only ContentSize, the Scrolled event, and the ScrollToAsync(x, y, animated)
// request pipeline:
//   - scroll_to_async raises scroll_to_requested (the IScrollViewController.ScrollToRequested event)
//     and routes a scroll_to_request to the handler — or PENDS it while no handler is attached,
//     flushing on attach (ScrollView.OnHandlerChangedCore + _pendingScrollToRequested). An
//     Orientation of `neither` suppresses the request entirely (C# returns early).
//   - The PLATFORM writes user scrolls back through i_scroll_view::set_horizontal_offset/
//     set_vertical_offset → set_scrolled_position (IScrollViewController.SetScrolledPosition): the
//     offsets update once and `scrolled` fires with the new position.
//   - scroll_finished() (the platform's completion ack) raises `scroll_to_completed` — the port's
//     synchronous stand-in for C#'s ScrollToAsync Task completion (_scrollCompletionSource), the same
//     async→event collapse the navigation transition uses. scroll_to_async returns whether a request
//     was dispatched (the Task object itself has no analog).
//
// measure ports the HANDLER-side CrossPlatformMeasure (ScrollViewHandler.iOS.cs — the variant C#
// routes every platform through "to normalize the behavior"): the content measures with the scrolling
// dimension(s) unconstrained (+inf per Orientation), within the Padding, then the result clamps to the
// incoming constraints and resolves against this view's own size requests. arrange ports
// LayoutExtensions.ArrangeContentUnbounded: the content arranges into the LARGER of the bounds and its
// desired size + Padding (it may exceed the viewport — that excess is the scrollable range), and
// ContentSize updates from the arranged content frame + margin (the ScrollView.ContentSizeChanged
// write). With no content, ContentSize is Zero (CrossPlatformMeasure).
//
// NOT ported (documented): ScrollToAsync(Element, ScrollToPosition, animated) — the element-coordinate
// walk (C# resolves it by REFLECTION over VisualElement X/Y; a frame-based walk can land later with
// the visual-tree work); read-only bindable keys for ScrollX/ScrollY/ContentSize (plain members here —
// observation goes through `scrolled`); the legacy Compatibility.Layout surface.

#include <functional>
#include <optional>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/i_scroll_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_to_request.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class scroll_view : public view<maui::core::i_scroll_view>, public maui::core::i_safe_area_view2
    {
    public:
        scroll_view()
        {
            this->set_style_target_type<scroll_view>();
        }

        // Shared bindable-property descriptors (ScrollView.OrientationProperty etc.).
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();
        static const maui::core::bindable_property<maui::core::scroll_orientation>& orientation_property();
        static const maui::core::bindable_property<maui::core::scroll_bar_visibility>&
        horizontal_scroll_bar_visibility_property();
        static const maui::core::bindable_property<maui::core::scroll_bar_visibility>&
        vertical_scroll_bar_visibility_property();

        // maui::controls::scroll_view::safe_area_edges_property <=
        // Microsoft.Maui.Controls.ScrollView.SafeAreaEdgesProperty
        static const maui::core::bindable_property<maui::core::safe_area_edges>& safe_area_edges_property();

        // ---- SafeAreaEdges (control-only; ScrollView.SafeAreaEdges) ----
        [[nodiscard]] maui::core::safe_area_edges safe_area_edges() const
        {
            return safe_area_edges_.get();
        }
        void set_safe_area_edges(maui::core::safe_area_edges value)
        {
            safe_area_edges_.set(value);
        }

        void set_safe_area_insets(const maui::core::thickness& value) override;
        [[nodiscard]] maui::core::safe_area_regions get_safe_area_regions_for_edge(int edge) const override;

        // C# `MauiScrollView.SystemAdjustedContentInset` — UIKit's OWN computed contentInset. It decides
        // which of MauiScrollView's two branches runs (MauiScrollView.cs:383-386), because UIKit only sets
        // it when the content OVERFLOWS the scroll view; when the content FITS it stays zero and the scroll
        // view must inset its content ITSELF. The native handler pushes it here — the port arranges
        // cross-platform, so this platform fact has to reach the control (same documented deviation as
        // set_safe_area_insets). Headless never pushes ⇒ zero ⇒ the manual branch over zero insets ⇒ no-op.
        void set_system_adjusted_content_inset(const maui::core::thickness& value)
        {
            system_adjusted_content_inset_ = value;
        }

        // C# MauiScrollView._safeArea after ValidateSafeArea + the line-389 gate. Zero unless a native host
        // pushed real insets, so every headless path is unaffected.
        [[nodiscard]] maui::core::thickness effective_safe_area() const;

        // True when UIKit already applied the inset via its own contentInset (the content overflows). Then
        // the content arranges at 0-origin with only the SIZE reduced — UIKit supplies the visual offset,
        // and re-adding the origin would double it.
        [[nodiscard]] bool system_applied_the_inset() const
        {
            return !system_adjusted_content_inset_.is_empty();
        }

        // ---- events ----
        // C# ScrollView.Scrolled (ScrolledEventArgs.ScrollX/ScrollY).
        maui::core::event<double, double> scrolled;
        // C# IScrollViewController.ScrollToRequested (instant = !ShouldAnimate).
        maui::core::event<maui::core::scroll_to_request> scroll_to_requested;
        // The ScrollToAsync Task-completion stand-in: raised by scroll_finished() (see header).
        maui::core::event<> scroll_to_completed;

        // ---- Content (non-owning; the same logical-child recipe as the other content hosts) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return content_;
        }
        void set_content(maui::core::i_view& value)
        {
            set_content(&value);
        }
        void set_content(maui::core::i_view* value)
        {
            if (content_ == value)
            {
                return;
            }
            if (auto* old_child = dynamic_cast<element*>(content_))
            {
                detach_logical_child(*old_child);
            }
            content_ = value;
            if (auto* new_child = dynamic_cast<element*>(content_))
            {
                attach_logical_child(*new_child);
            }
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("set_content");
            }
        }

        // ---- i_padding ----
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_.get();
        }
        void set_padding(maui::core::thickness value)
        {
            padding_.set(value);
        }

        // ---- Orientation / scroll-bar visibilities ----
        [[nodiscard]] maui::core::scroll_orientation orientation() const override
        {
            return orientation_.get();
        }
        void set_orientation(maui::core::scroll_orientation value)
        {
            orientation_.set(value);
        }

        [[nodiscard]] maui::core::scroll_bar_visibility horizontal_scroll_bar_visibility() const override
        {
            return horizontal_scroll_bar_visibility_.get();
        }
        void set_horizontal_scroll_bar_visibility(maui::core::scroll_bar_visibility value)
        {
            horizontal_scroll_bar_visibility_.set(value);
        }

        [[nodiscard]] maui::core::scroll_bar_visibility vertical_scroll_bar_visibility() const override
        {
            return vertical_scroll_bar_visibility_.get();
        }
        void set_vertical_scroll_bar_visibility(maui::core::scroll_bar_visibility value)
        {
            vertical_scroll_bar_visibility_.set(value);
        }

        // ---- the read-only scroll position (ScrollView.ScrollX/ScrollY) ----
        [[nodiscard]] double scroll_x() const
        {
            return scroll_x_;
        }
        [[nodiscard]] double scroll_y() const
        {
            return scroll_y_;
        }

        // C# IScrollViewController.SetScrolledPosition: update both offsets once and fire Scrolled
        // (a no-op when the position is unchanged).
        void set_scrolled_position(double x, double y)
        {
            if (scroll_x_ == x && scroll_y_ == y)
            {
                return;
            }
            scroll_x_ = x;
            scroll_y_ = y;
            scrolled.raise(x, y);
        }

        // ---- ScrollToAsync(x, y, animated) ----
        // Returns whether a request was dispatched (false when Orientation is `neither` — C# returns a
        // completed Task without raising anything). Completion surfaces via `scroll_to_completed`.
        bool scroll_to_async(double x, double y, bool animated)
        {
            if (orientation() == maui::core::scroll_orientation::neither)
            {
                return false;
            }
            on_scroll_to_requested({.horizontal_offset = x, .vertical_offset = y, .instant = !animated});
            return true;
        }

        // ---- i_scroll_view (the platform-facing surface) ----
        [[nodiscard]] maui::graphics::size content_size() const override
        {
            return content_size_;
        }
        [[nodiscard]] double horizontal_offset() const override
        {
            return scroll_x_;
        }
        void set_horizontal_offset(double value) override
        {
            if (scroll_x_ != value)
            {
                set_scrolled_position(value, scroll_y_);
            }
        }
        [[nodiscard]] double vertical_offset() const override
        {
            return scroll_y_;
        }
        void set_vertical_offset(double value) override
        {
            if (scroll_y_ != value)
            {
                set_scrolled_position(scroll_x_, value);
            }
        }
        // C# IScrollView.ScrollFinished → SendScrollFinished (the Task completion stand-in).
        void scroll_finished() override
        {
            scroll_to_completed.raise();
        }
        // C# IScrollView.RequestScrollTo: hand the request straight to the handler.
        void request_scroll_to(double horizontal, double vertical, bool instant) override
        {
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("request_scroll_to",
                                        maui::core::scroll_to_request{.horizontal_offset = horizontal,
                                                                      .vertical_offset = vertical,
                                                                      .instant = instant});
            }
        }

        // Flush a request pended while detached (ScrollView.OnHandlerChangedCore).
        void set_handler(std::shared_ptr<maui::core::i_element_handler> value) override;

        // ---- layout pass (see the header comment for the oracles) ----
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (auto* child = dynamic_cast<element*>(content_))
            {
                visit(*child);
            }
        }

        // Generic mount (app_host): re-fire "set_content" so the now-attached handler hosts the scrolled
        // content's native view (the construction-order replay of set_content's command).
        void mount_into_handler() override
        {
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("set_content");
            }
        }

    private:
        // C# OnScrollToRequested: raise the controller event, then dispatch to the handler — or pend
        // the (single, latest-wins) request until one attaches.
        void on_scroll_to_requested(const maui::core::scroll_to_request& request)
        {
            scroll_to_requested.raise(request);
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("request_scroll_to", request);
            }
            else
            {
                pending_scroll_to_ = request;
            }
        }

        // C# MauiScrollView._safeArea inputs: the realized UIKit insets, and UIKit's own contentInset
        // adjustment (which selects the manual-vs-system branch). Both pushed by the native handler.
        maui::core::thickness safe_area_insets_;
        maui::core::thickness system_adjusted_content_inset_;
        maui::core::i_view* content_ = nullptr; // NON-owning: the caller owns the content's lifetime
        maui::graphics::size content_size_;     // read-only ContentSize (see header)
        double scroll_x_ = 0;                   // read-only ScrollX
        double scroll_y_ = 0;                   // read-only ScrollY
        std::optional<maui::core::scroll_to_request> pending_scroll_to_; // _pendingScrollToRequested
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
        maui::core::property<maui::core::scroll_orientation> orientation_{*this, orientation_property()};
        maui::core::property<maui::core::scroll_bar_visibility> horizontal_scroll_bar_visibility_{
            *this, horizontal_scroll_bar_visibility_property()};
        maui::core::property<maui::core::scroll_bar_visibility> vertical_scroll_bar_visibility_{
            *this, vertical_scroll_bar_visibility_property()};
        maui::core::property<maui::core::safe_area_edges> safe_area_edges_{*this, safe_area_edges_property()};
    };
} // namespace maui::controls
