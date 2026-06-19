#pragma once
// maui::samples::custom_layout_page — ports CustomLayoutPage.xaml (+ CustomLayoutPage.xaml.cs).
//
// The C# sample defines a developer-authored custom layout — `DockLayout` — entirely in user code
// (the gallery's point is "you can write your own ILayoutManager"): a `Layout` subclass whose
// `CreateLayoutManager()` returns a private `DockLayoutManager : LayoutManager` that docks each child
// to an edge (Left/Top/Right/Bottom) via an attached `DockLayout.Dock` property, with the last child
// optionally filling the remainder (`LastChildFill`). The XAML then drops six docked buttons into it
// (Top, Bottom, Left, Left, Right, Right) with LastChildFill="False".
//
// This page ports that faithfully: it owns a SELF-CONTAINED custom layout — `dock_layout`, a
// layout<maui::core::i_layout> with an embedded `dock_layout_manager : maui::layouts::layout_manager`
// whose measure/arrange_children mirror DockLayoutManager line-for-line — plus per-child Dock attached
// storage (keyed on the child pointer, the same store shape absolute_layout/grid use). The custom
// layer is the demonstration; it leans only on the public M3 layout-manager seam
// (maui::layouts::i_layout_manager + the layout<> base), exactly as a MAUI app author would.
//
// The page OWNS its whole element tree (the value_controls_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same controls directly.
//
// Demonstrated:
//   - a real custom i_layout_manager (DockLayoutManager) ported from user C#, not a built-in;
//   - per-child Dock attached storage + LastChildFill, driving measure + arrange exactly as C# does;
//   - six docked buttons (Top/Bottom/Left/Left/Right/Right) with LastChildFill = false.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <unordered_map>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/layout.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/layouts/i_layout_manager.hpp"
#include "maui/layouts/layout_manager.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    // ---- the developer-authored custom layout (ports CustomLayoutPage.xaml.cs: enum Dock + DockLayout) ----

    // C# `public enum Dock { Left, Top, Right, Bottom }`.
    enum class dock : std::uint8_t
    {
        left = 0,
        top = 1,
        right = 2,
        bottom = 3,
    };

    // C# `public class DockLayout : Layout`. A user-space layout control: it derives the port's layout<>
    // base over i_layout (children + padding + the manager seam), adds the attached `Dock` per-child store
    // and the `LastChildFill` knob, and returns its own manager from create_layout_manager(). This is the
    // whole point of the sample — a custom ILayoutManager written in app code.
    class dock_layout : public maui::controls::layout<maui::core::i_layout>
    {
    public:
        dock_layout() : layout(padding_property())
        {
            this->set_style_target_type<dock_layout>(); // implicit / class style match
        }

        // The shared padding descriptor (C# Layout.PaddingProperty; every concrete layout supplies its own
        // static "padding" thickness descriptor — see vertical_stack_layout::padding_property()).
        static const maui::core::bindable_property<maui::core::thickness>& padding_property()
        {
            static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
            return descriptor;
        }

        // C# DockLayout.LastChildFillProperty (default true). The XAML sets it False; honored in arrange.
        [[nodiscard]] bool last_child_fill() const
        {
            return last_child_fill_;
        }
        void set_last_child_fill(bool value)
        {
            last_child_fill_ = value;
            this->invalidate_measure();
        }

        // C# attached DockLayout.Dock (default Dock.Left). Per-child, keyed on the child pointer (the same
        // attached-store shape absolute_layout/grid use). Read by the manager during arrange.
        [[nodiscard]] dock get_dock(const maui::core::i_view& view) const
        {
            const auto found = dock_.find(&view);
            return found == dock_.end() ? dock::left : found->second;
        }
        void set_dock(maui::core::i_view& view, dock value)
        {
            dock_[&view] = value;
            this->invalidate_measure(); // a dock change re-lays-out (C# attached-property change → invalidate)
        }

        // Prune the per-child store for departed children (C# OnRemove/OnClear analog).
        void remove_at(int index) override
        {
            const maui::core::i_view* const removed = &this->at(index);
            layout::remove_at(index);
            dock_.erase(removed);
        }
        void clear() override
        {
            layout::clear();
            dock_.clear();
        }

    protected:
        [[nodiscard]] std::unique_ptr<maui::layouts::i_layout_manager> create_layout_manager() override;

    private:
        bool last_child_fill_ = true; // DockLayout.LastChildFillProperty default
        std::unordered_map<const maui::core::i_view*, dock> dock_;
    };

    // C# `class DockLayoutManager : LayoutManager` — the custom algorithm. measure/arrange_children are
    // ported line-for-line from CustomLayoutPage.xaml.cs (visibility-gated child walk; per-dock edge
    // bookkeeping; the LastChildFill short-circuit on the final visible child).
    class dock_layout_manager : public maui::layouts::layout_manager
    {
    public:
        explicit dock_layout_manager(dock_layout& owner) : layout_manager(owner), owner_(&owner)
        {
        }

        // C# DockLayoutManager.Measure.
        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override
        {
            double width = 0;
            double height = 0;
            double final_width = 0;
            double final_height = 0;

            const int n = owner_->count();
            for (int i = 0; i < n; ++i)
            {
                maui::core::i_view& child = owner_->at(i);
                if (child.visibility() != maui::core::visibility::visible)
                {
                    continue;
                }

                const maui::graphics::size request = child.measure(width_constraint, height_constraint);
                switch (owner_->get_dock(child))
                {
                    case dock::left:
                    case dock::right:
                        width += request.width;
                        final_width = std::max(final_width, width);
                        final_height = std::max(final_height, height + request.height);
                        break;
                    case dock::top:
                    case dock::bottom:
                        height += request.height;
                        final_width = std::max(final_width, width + request.width);
                        final_height = std::max(final_height, height);
                        break;
                }
            }
            return {final_width, final_height};
        }

        // C# DockLayoutManager.ArrangeChildren.
        maui::graphics::size arrange_children(const maui::graphics::rect& bounds) override
        {
            double x = bounds.x;
            double y = bounds.y;
            double width = bounds.width;
            double height = bounds.height;
            const maui::graphics::size size_request{}; // C# returns Size.Zero throughout

            const int n = owner_->count();
            int i = 0;
            for (int idx = 0; idx < n; ++idx)
            {
                maui::core::i_view& child = owner_->at(idx);
                if (child.visibility() != maui::core::visibility::visible)
                {
                    continue;
                }
                ++i;

                double child_x = 0;
                double child_y = 0;
                const maui::graphics::size request = child.desired_size(); // == sizeRequest read in C#
                double child_width = std::min(width, request.width);
                double child_height = std::min(height, request.height);

                const bool last_item = (i == n);
                if (last_item && owner_->last_child_fill())
                {
                    child.arrange(maui::graphics::rect(x, y, width, height));
                    return size_request;
                }

                switch (owner_->get_dock(child))
                {
                    case dock::left:
                        child_x = x;
                        child_y = y;
                        child_height = height;
                        x += child_width;
                        width -= child_width;
                        break;
                    case dock::top:
                        child_x = x;
                        child_y = y;
                        child_width = width;
                        y += child_height;
                        height -= child_height;
                        break;
                    case dock::right:
                        child_x = x + width - child_width;
                        child_y = y;
                        child_height = height;
                        width -= child_width;
                        break;
                    case dock::bottom:
                        child_x = x;
                        child_y = y + height - child_height;
                        child_width = width;
                        height -= child_height;
                        break;
                }

                child.arrange(maui::graphics::rect(child_x, child_y, child_width, child_height));
            }
            return size_request;
        }

    private:
        dock_layout* owner_;
    };

    inline std::unique_ptr<maui::layouts::i_layout_manager> dock_layout::create_layout_manager()
    {
        return std::make_unique<dock_layout_manager>(*this);
    }

    // ---- the page (ports CustomLayoutPage.xaml: six docked buttons in the DockLayout) ----

    class custom_layout_page
    {
    public:
        custom_layout_page()
        {
            page_.set_title("Custom Layout");

            // LastChildFill="False" — every child docks, none fills the remainder.
            dock_.set_last_child_fill(false);

            // <Button DockLayout.Dock="Top" Text="Top" HeightRequest="50"/>
            top_.set_text("Top");
            top_.set_height_request(50);
            dock_.add(top_);
            dock_.set_dock(top_, dock::top);

            // <Button DockLayout.Dock="Bottom" Text="Bottom" HeightRequest="50"/>
            bottom_.set_text("Bottom");
            bottom_.set_height_request(50);
            dock_.add(bottom_);
            dock_.set_dock(bottom_, dock::bottom);

            // <Button DockLayout.Dock="Left" Text="Left" WidthRequest="60"/> (x2)
            left_a_.set_text("Left");
            left_a_.set_width_request(60);
            dock_.add(left_a_);
            dock_.set_dock(left_a_, dock::left);

            left_b_.set_text("Left");
            left_b_.set_width_request(60);
            dock_.add(left_b_);
            dock_.set_dock(left_b_, dock::left);

            // <Button DockLayout.Dock="Right" Text="Right" WidthRequest="80"/> (x2)
            right_a_.set_text("Right");
            right_a_.set_width_request(80);
            dock_.add(right_a_);
            dock_.set_dock(right_a_, dock::right);

            right_b_.set_text("Right");
            right_b_.set_width_request(80);
            dock_.add(right_b_);
            dock_.set_dock(right_b_, dock::right);

            page_.set_content(dock_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the buttons, then the dock layout, then the
        // page), then re-host the tree built in the ctor (gallery_attach.hpp). The dock_layout reuses the
        // layout host command path ("add"), so gallery_rehost_layout mirrors its children onto the panel.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            // dock_layout is a brand-new user type, so its default handler isn't registered the way the
            // built-in controls self-register. Register layout_handler for it once (idempotent) so
            // attach_handler(dock_) resolves the same panel host every built-in layout uses.
            static const bool registered = [] {
                maui::core::register_handler<dock_layout, maui::core::layout_handler>(
                    maui::core::default_handler_registry());
                return true;
            }();
            (void)registered;

            auto one = [&app](auto& v, const char* n) {
                try
                {
                    app.attach_handler(v);
                }
                catch (const std::exception& e)
                {
                    std::fprintf(stderr, "[gallery] skip %s: %s\n", n, e.what());
                }
            };

            one(top_, "top_");
            one(bottom_, "bottom_");
            one(left_a_, "left_a_");
            one(left_b_, "left_b_");
            one(right_a_, "right_a_");
            one(right_b_, "right_b_");
            one(dock_, "dock_");
            one(page_, "page_");

            gallery_rehost_layout(dock_); // dock layout hosts its six buttons
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] dock_layout& docker()
        {
            return dock_;
        }
        [[nodiscard]] maui::controls::button& top_button()
        {
            return top_;
        }
        [[nodiscard]] maui::controls::button& bottom_button()
        {
            return bottom_;
        }

    private:
        maui::controls::content_page page_;
        dock_layout dock_;
        maui::controls::button top_;
        maui::controls::button bottom_;
        maui::controls::button left_a_;
        maui::controls::button left_b_;
        maui::controls::button right_a_;
        maui::controls::button right_b_;
    };
} // namespace maui::samples
