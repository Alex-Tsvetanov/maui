#pragma once
// maui::samples::ios_scroll_view_page — ports iOSScrollViewPage (Platform-Specifics gallery).
//
// The C# page is a FlyoutPage whose DETAIL pane is a ScrollView (with the iOS attached property
// ios:ScrollView.ShouldDelayContentTouches="false") over a Slider and two Buttons; its Flyout pane is a
// blue "Menu" ContentPage. The ShouldDelayContentTouches knob governs whether the inner Slider responds to
// a touch immediately vs after the UIScrollView's delay.
//
// The C# code-behind's two buttons:
//   - "Toggle ScrollView DelayContentTouches" → OnButtonClicked flips ShouldDelayContentTouches (ported
//     verbatim via the platform_configuration::ios_specific::scroll_view knob).
//   - "Return to Platform-Specifics List" → OnReturnButtonClicked calls Navigation.PopAsync(). This
//     standalone gallery leaf owns no NavigationPage host, so the return button is a safe no-op.
//
// The shared ios_scroll_view.xaml is captured at REST and DEGRADES the FlyoutPage to just its DETAIL pane —
// a plain ContentPage over the ScrollView — because the "Menu" flyout + the PopAsync return are page chrome
// (a FlyoutPage renders a Mac Catalyst split view + sidebar-toggle that MAUI's resting capture does not
// show). This twin mirrors that degraded resting shape exactly: page() is a ContentPage whose content is
// the ScrollView. It is backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer
// and hosts page() in a window; the headless/apple/ios test trees exercise the same wiring directly.

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/scroll_orientation.hpp"

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/scroll_view.hpp"

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

            // The ScrollView's default Orientation is Vertical; set explicitly so the demo content scrolls.
            scroller_.set_orientation(maui::core::scroll_orientation::vertical);

            // iOSSpecific ScrollView knob, applied at build time through the ported config accessor
            // (scroller.on<ios>()) + the free-function knob set — exactly the C# XAML attached property:
            //   ios:ScrollView.ShouldDelayContentTouches="false".
            ios_scroll::set_should_delay_content_touches(scroller_.on<pc::ios>(), false);

            // The inner Slider — the control whose touch responsiveness the knob governs on iOS (XAML Value=50).
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
            // this standalone gallery leaf has no NavigationPage host, so the click is a safe no-op.
            return_button_.set_text("Return to Platform-Specifics List");

            content_stack_.set_spacing(20);
            content_stack_.add(inner_slider_);
            content_stack_.add(toggle_button_);
            content_stack_.add(return_button_);
            scroller_.set_content(content_stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::scroll_view& scroller()
        {
            return scroller_;
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
        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout content_stack_;
        maui::controls::slider inner_slider_;
        maui::controls::button toggle_button_;
        maui::controls::button return_button_;
    };
} // namespace maui::samples
