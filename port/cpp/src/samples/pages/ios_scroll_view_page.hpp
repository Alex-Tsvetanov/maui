#pragma once
// maui::samples::ios_scroll_view_page — ports iOSScrollViewPage.xaml (+ iOSScrollViewPage.xaml.cs).
//
// A self-contained, code-first demo of the iOSSpecific ScrollView.ShouldDelayContentTouches knob
// (W2-24). It mirrors the C# gallery page (Pages/PlatformSpecifics/iOS/iOSScrollViewPage), which is a
// FlyoutPage whose:
//   - FLYOUT pane is a titled "Menu" content_page (the C# <ContentPage Title="Menu" BackgroundColor=
//     "Blue"/>), and
//   - DETAIL pane is a content_page hosting a ScrollView on which the iOS-specific attached knob is
//     applied at build time — ScrollView.ShouldDelayContentTouches = false
//     (ios:ScrollView.ShouldDelayContentTouches="false") — wrapping a Slider and two Buttons.
//
// The demonstrated point is the platform-configuration SURFACE: the knob is exercised through the
// ported `element.on<ios>()` config accessor + the ios_specific::scroll_view free-function knob set
// (configuration.hpp / ios_specific/scroll_view.hpp). On iOS ShouldDelayContentTouches=false makes the
// inner controls (e.g. the Slider) respond to a touch immediately rather than after the UIScrollView's
// touch-delay window; on the headless/AppKit backends the knob is stored-inert (the value round-trips
// through the element's platform-spec store but no native UIScrollView delay exists). Either way the
// ScrollView + its content render and the config calls compile + apply.
//
// The C# code-behind's two buttons:
//   - "Toggle ScrollView DelayContentTouches" → OnButtonClicked flips ShouldDelayContentTouches, ported
//     verbatim through the config accessor;
//   - "Return to Platform-Specifics List" → OnReturnButtonClicked calls Navigation.PopAsync(). This page
//     owns no NavigationPage host (it is the gallery leaf), so the return button is wired to a no-op-safe
//     status update instead (see note); the navigation pop has no headless-safe standalone analog here.
//
// The page OWNS its whole element tree (the tabbed_flyout_page pattern). flyout_page derives from
// content_page, so page() returns the flyout root as a content_page& (the required gallery contract)
// while still mirroring the C# FlyoutPage shape. It is backend-agnostic — a sample main attaches handlers
// bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios test trees
// exercise the same wiring directly.
//
// note: the C# flyout menu page is BackgroundColor="Blue"; the port models a page background through a
//       solid_paint on the page's content (content_page has no direct BackgroundColor setter beyond the
//       view background surface), so the blue is applied to the menu page's content label background as a
//       faithful color stand-in.

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/scroll_view.hpp"

#include <memory>

namespace maui::samples
{
    class ios_scroll_view_page
    {
    public:
        ios_scroll_view_page()
        {
            namespace pc = maui::controls::platform_configuration;
            namespace ios_scroll = pc::ios_specific::scroll_view;

            page_.set_title("ScrollView DelayContentTouches");

            // ---- the flyout: a titled "Menu" page, tinted blue (the C# Title="Menu" BackgroundColor=
            // "Blue"). FlyoutPage requires the flyout to carry a Title. ----
            menu_page_.set_title("Menu");
            menu_label_.set_text("Menu");
            // Blue background stand-in (see header note): there is no content_page BackgroundColor setter,
            // so the page's content label carries the blue via a solid_paint.
            menu_label_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));
            menu_page_.set_content(menu_label_);

            // ---- the detail: a content_page hosting the ScrollView ----
            // The scroll_view's default Orientation is Vertical; set explicitly so the demo content scrolls.
            scroller_.set_orientation(maui::core::scroll_orientation::vertical);

            // iOSSpecific ScrollView knob, applied at build time through the ported config accessor
            // (scroller.on<ios>()) + the free-function knob set — exactly the C# XAML attached property:
            //   ios:ScrollView.ShouldDelayContentTouches="false".
            ios_scroll::set_should_delay_content_touches(scroller_.on<pc::ios>(), false);

            // The inner Slider — the control whose touch responsiveness the knob governs on iOS.
            inner_slider_.set_minimum(0);
            inner_slider_.set_maximum(100);
            inner_slider_.set_value(50);

            // "Toggle ScrollView DelayContentTouches" — ports OnButtonClicked verbatim.
            toggle_button_.set_text("Toggle ScrollView DelayContentTouches");
            toggle_button_.clicked.connect([this] {
                namespace pcfg = maui::controls::platform_configuration;
                namespace ios_sv = pcfg::ios_specific::scroll_view;
                const bool current = ios_sv::should_delay_content_touches(scroller_.on<pcfg::ios>());
                ios_sv::set_should_delay_content_touches(scroller_.on<pcfg::ios>(), !current);
            });

            // "Return to Platform-Specifics List" — the C# OnReturnButtonClicked calls Navigation.PopAsync();
            // this standalone gallery page has no NavigationPage host, so the click updates a status label
            // (the pop has no headless-safe standalone analog; see header note).
            return_button_.set_text("Return to Platform-Specifics List");
            return_button_.clicked.connect([this] { menu_label_.set_text("Return requested (no nav host)"); });

            content_stack_.set_spacing(20);
            content_stack_.add(inner_slider_);
            content_stack_.add(toggle_button_);
            content_stack_.add(return_button_);
            scroller_.set_content(content_stack_);
            detail_page_.set_content(scroller_);

            // ---- the flyout page over both panes ----
            page_.set_flyout(&menu_page_);
            page_.set_detail(&detail_page_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_; // flyout_page IS-A content_page (the required gallery contract)
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::scroll_view& scroller()
        {
            return scroller_;
        }
        [[nodiscard]] maui::controls::content_page& menu_page()
        {
            return menu_page_;
        }
        [[nodiscard]] maui::controls::content_page& detail_page()
        {
            return detail_page_;
        }
        [[nodiscard]] maui::controls::slider& inner_slider()
        {
            return inner_slider_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }
        [[nodiscard]] maui::controls::button& return_button()
        {
            return return_button_;
        }

    private:
        maui::controls::flyout_page page_;
        maui::controls::content_page menu_page_;
        maui::controls::label menu_label_;
        maui::controls::content_page detail_page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout content_stack_;
        maui::controls::slider inner_slider_;
        maui::controls::button toggle_button_;
        maui::controls::button return_button_;
    };
} // namespace maui::samples
