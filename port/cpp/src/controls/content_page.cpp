// maui::controls::content_page — out-of-line definitions: the shared bindable-property descriptors, the
// measure/arrange content layout (LayoutExtensions.MeasureContent/ArrangeContent), and the default-
// handler self-registration. See content_page.hpp.

#include "maui/controls/content_page.hpp"

#include <string>

#include "maui/controls/element.hpp"                                  // --- W2-24: the page-probe element type
#include "maui/controls/flyout_page.hpp"                              // --- W2-24: the parent-is-page probe
#include "maui/controls/navigation_page.hpp"                          // --- W2-24
#include "maui/controls/platform_configuration/ios_specific/page.hpp" // --- W2-24: the knob store face
#include "maui/controls/tabbed_page.hpp"                              // --- W2-24
#include "maui/core/bindable_property.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::core::thickness>& content_page::padding_property()
    {
        // C# Page.PaddingDefaultValueCreator returns Thickness(0); the typed default T{} is all-zero.
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& content_page::title_property()
    {
        // C# Page.TitleProperty default is null → the empty string here.
        static const maui::core::bindable_property<std::string> descriptor{"title", std::string{}};
        return descriptor;
    }

    // C# LayoutExtensions.MeasureContent (this M4c cut omits the explicit Width/Height short-circuit, as
    // the control has no bindable WidthRequest/HeightRequest yet — the deferred VisualElement surface):
    // measure the content within the padding, then add the padding back. With no content, the measured
    // size is the padding only.
    maui::graphics::size content_page::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness inset = layout_inset();
        maui::graphics::size content_size{0, 0};
        if (content_ != nullptr)
        {
            content_size = content_->measure(width_constraint - inset.horizontal_thickness(),
                                             height_constraint - inset.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        desired_size_ = measured;
        return measured;
    }

    // C# ContentPage.ArrangeOverride/CrossPlatformArrange + LayoutExtensions.ArrangeContent: record the
    // frame, size the native host panel, then arrange the single content within the padding inset. With
    // no content there is nothing to arrange (ArrangeContent returns early).
    maui::graphics::size content_page::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds); // size/position the host panel
        }
        if (content_ != nullptr)
        {
            const maui::core::thickness inset = layout_inset();
            const maui::graphics::rect target{bounds.x + inset.left, bounds.y + inset.top,
                                              bounds.width - inset.horizontal_thickness(),
                                              bounds.height - inset.vertical_thickness()};
            content_->arrange(target);
        }
        return {bounds.width, bounds.height};
    }

    // --- platform configuration (W2-24): the iOSSpecific Page faces --------------------------------------

    namespace
    {
        namespace ios_page = platform_configuration::ios_specific::page;

        // The C# `Parent is Page` probe — the port's declared page set (no shared page base; the same
        // declared-list substitute as platform_configuration::page_element).
        bool is_page(const element& candidate)
        {
            return dynamic_cast<const content_page*>(&candidate) != nullptr ||
                   dynamic_cast<const navigation_page*>(&candidate) != nullptr ||
                   dynamic_cast<const tabbed_page*>(&candidate) != nullptr ||
                   dynamic_cast<const flyout_page*>(&candidate) != nullptr;
        }

        // Page.cs's parent-redirect guard, shared by all three IiOSPageSpecifics getters: the parent page
        // wins when it has the HOME-INDICATOR knob set (yes — the status-bar getters also probe the
        // home-indicator key; an oracle quirk ported verbatim).
        const element* redirect_target(const element& self)
        {
            const element* parent = self.logical_parent();
            if (parent != nullptr && is_page(*parent) &&
                parent->has_platform_spec(ios_page::prefers_home_indicator_auto_hidden_key))
            {
                return parent;
            }
            return &self;
        }
    } // namespace

    // C# Page.IiOSPageSpecifics.IsHomeIndicatorAutoHidden.
    bool content_page::is_home_indicator_auto_hidden() const
    {
        return ios_page::get_prefers_home_indicator_auto_hidden(*redirect_target(*this));
    }

    // C# Page.IiOSPageSpecifics.PrefersStatusBarHiddenMode ((int)StatusBarHiddenMode).
    int content_page::prefers_status_bar_hidden_mode() const
    {
        return static_cast<int>(ios_page::get_prefers_status_bar_hidden(*redirect_target(*this)));
    }

    // C# Page.IiOSPageSpecifics.PreferredStatusBarUpdateAnimationMode ((int)UIStatusBarAnimation).
    int content_page::preferred_status_bar_update_animation_mode() const
    {
        return static_cast<int>(ios_page::get_preferred_status_bar_update_animation(*redirect_target(*this)));
    }

    // C# Page.ISafeAreaView.IgnoreSafeArea => !On<iOS>().UsingSafeArea().
    bool content_page::ignore_safe_area() const
    {
        return !ios_page::get_use_safe_area(*this);
    }

    // C# Page.ISafeAreaView2.SafeAreaInsets set => On<iOS>().SetSafeAreaInsets(value) (the native host's
    // write-back channel; the value lands in the read-only "ios.Page.SafeAreaInsets" store).
    void content_page::set_safe_area_insets(const maui::core::thickness& value)
    {
        ios_page::set_safe_area_insets(*this, value);
    }

    // The effective layout inset: Padding, plus the realized safe-area insets when the page honors them
    // (UseSafeArea set) — the cross-platform face of C# MauiView.AdjustForSafeArea (which insets the
    // bounds before CrossPlatformArrange; the port folds it into MeasureContent/ArrangeContent's inset).
    maui::core::thickness content_page::layout_inset() const
    {
        maui::core::thickness inset = padding();
        if (!ignore_safe_area())
        {
            inset = inset + ios_page::get_safe_area_insets(*this);
        }
        return inset;
    }
    // --- end platform configuration (W2-24) ---------------------------------------------------------------
} // namespace maui::controls

// Self-register the default handler for content_page (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::content_page, maui::core::content_page_handler)
