#pragma once
// maui::controls::tabbed_page  <=  Microsoft.Maui.Controls.TabbedPage
//
// A multi_page<content_page> shown as a row of tabs: each child page is one tab (its Title is the tab
// text) and the current page fills the content area. Ported from TabbedPage.cs (+ TabbedPage.Mapper.cs
// for the handler keys): the bar styling — BarBackgroundColor / BarTextColor (the IBarElement pair) and
// SelectedTabColor / UnselectedTabColor — is bindable, with a *_set_ flag distinguishing "never set" so
// the native bar keeps its system default (C# default(Color) = null; the navigation_page bar-color
// convention). CreateDefault makes a plain page titled with the item's text (TabbedPage.CreateDefault:
// new Page { Title = item.ToString() }).
//
// The handler seam is maui::core::i_tabbed_view (see that header: C#'s empty ITabbedView widened with
// the state the control-typed C# mappers read, since the reflection-free handler cannot reach this
// controls type). on_tab_selected is the native→virtual selection sync — a user tab tap makes that
// page current. BarBackground (Brush — IBarElement.BarBackground) is a brush fill the bar overlays as a
// CALayer (iOS paints; AppKit/headless mirror only); the Android/Windows platform-specifics are out of
// scope (documented in STATUS.md).

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/multi_page.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class tabbed_page : public multi_page<content_page>, public maui::core::i_tabbed_view
    {
    public:
        tabbed_page()
        {
            this->set_style_target_type<tabbed_page>(); // implicit / class style match
        }

        // Shared bindable-property descriptors (one instance per type, like TabbedPage.*Property).
        static const maui::core::bindable_property<maui::graphics::color>& bar_background_color_property();
        static const maui::core::bindable_property<std::shared_ptr<brush>>& bar_background_property();
        static const maui::core::bindable_property<maui::graphics::color>& bar_text_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& selected_tab_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& unselected_tab_color_property();

        // ---- the bar styling (each routes through on_property_changed → the handler's mapper key) ----
        [[nodiscard]] maui::graphics::color bar_background_color() const
        {
            return bar_background_color_.get();
        }
        void set_bar_background_color(maui::graphics::color value)
        {
            bar_background_color_set_ = true;
            bar_background_color_.set(value);
        }

        // C# TabbedPage.BarBackground (IBarElement.BarBackground): the bar fill as a Brush. The control
        // owns the brush (so it inherits this page's BindingContext — C#
        // VisualElement.SetInheritedBindingContext(Background, …)); a borrowed pointer reaches the handler
        // through tab_bar_background_brush(). Returns a raw borrow (the property owns the shared_ptr).
        [[nodiscard]] brush* bar_background() const
        {
            return bar_background_.get().get();
        }
        void set_bar_background(std::shared_ptr<brush> value)
        {
            bar_background_set_ = value != nullptr;
            if (value)
            {
                value->set_inherited_binding_context(this->raw_binding_context());
            }
            bar_background_.set(std::move(value));
        }

        [[nodiscard]] maui::graphics::color bar_text_color() const
        {
            return bar_text_color_.get();
        }
        void set_bar_text_color(maui::graphics::color value)
        {
            bar_text_color_set_ = true;
            bar_text_color_.set(value);
        }

        [[nodiscard]] maui::graphics::color selected_tab_color() const
        {
            return selected_tab_color_.get();
        }
        void set_selected_tab_color(maui::graphics::color value)
        {
            selected_tab_color_set_ = true;
            selected_tab_color_.set(value);
        }

        [[nodiscard]] maui::graphics::color unselected_tab_color() const
        {
            return unselected_tab_color_.get();
        }
        void set_unselected_tab_color(maui::graphics::color value)
        {
            unselected_tab_color_set_ = true;
            unselected_tab_color_.set(value);
        }

        // ---- i_tabbed_view (the handler reads these to build the native tab chrome) ----
        [[nodiscard]] std::vector<maui::core::i_view*> tabbed_pages() const override
        {
            std::vector<maui::core::i_view*> pages;
            pages.reserve(children().size());
            for (content_page* const page : children())
            {
                pages.push_back(page);
            }
            return pages;
        }

        [[nodiscard]] std::vector<std::string> tabbed_titles() const override
        {
            std::vector<std::string> titles;
            titles.reserve(children().size());
            for (content_page* const page : children())
            {
                titles.emplace_back(page->title());
            }
            return titles;
        }

        [[nodiscard]] maui::core::i_view* tabbed_current_page() const override
        {
            return current_page();
        }

        void on_tab_selected(std::size_t index) override
        {
            if (index < children().size())
            {
                set_current_page(children()[index]);
            }
        }

        [[nodiscard]] std::optional<maui::graphics::color> tab_bar_background_color() const override
        {
            return bar_background_color_set_ ? std::optional{bar_background_color_.get()} : std::nullopt;
        }
        [[nodiscard]] std::optional<maui::graphics::color> tab_bar_text_color() const override
        {
            return bar_text_color_set_ ? std::optional{bar_text_color_.get()} : std::nullopt;
        }
        [[nodiscard]] std::optional<maui::graphics::color> tab_selected_color() const override
        {
            return selected_tab_color_set_ ? std::optional{selected_tab_color_.get()} : std::nullopt;
        }
        [[nodiscard]] std::optional<maui::graphics::color> tab_unselected_color() const override
        {
            return unselected_tab_color_set_ ? std::optional{unselected_tab_color_.get()} : std::nullopt;
        }
        [[nodiscard]] std::optional<brush*> tab_bar_background_brush() const override
        {
            // A borrowed pointer (the control owns the brush via bar_background_); nullopt when never set
            // (C# default(Brush) = null) so the bar keeps its color-driven default.
            return bar_background_set_ ? std::optional{bar_background_.get().get()} : std::nullopt;
        }

    protected:
        // TabbedPage.CreateDefault: a plain page titled with the item's text.
        [[nodiscard]] std::shared_ptr<content_page> create_default(const std::string& item_text) override
        {
            auto page = std::make_shared<content_page>();
            if (!item_text.empty())
            {
                page->set_title(item_text);
            }
            return page;
        }

    private:
        maui::core::property<maui::graphics::color> bar_background_color_{*this, bar_background_color_property()};
        maui::core::property<std::shared_ptr<brush>> bar_background_{*this, bar_background_property()};
        maui::core::property<maui::graphics::color> bar_text_color_{*this, bar_text_color_property()};
        maui::core::property<maui::graphics::color> selected_tab_color_{*this, selected_tab_color_property()};
        maui::core::property<maui::graphics::color> unselected_tab_color_{*this, unselected_tab_color_property()};
        bool bar_background_color_set_ = false;
        bool bar_background_set_ = false;
        bool bar_text_color_set_ = false;
        bool selected_tab_color_set_ = false;
        bool unselected_tab_color_set_ = false;
    };
} // namespace maui::controls
