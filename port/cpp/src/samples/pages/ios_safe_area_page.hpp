#pragma once
// maui::samples::ios_safe_area_page — ports iOSSafeAreaPage.xaml
//
// The .NET MAUI PlatformSpecifics sample (Pages/PlatformSpecifics/iOS/iOSSafeAreaPage.xaml +
// .xaml.cs): a long Lorem-ipsum Label over a "Disable Use Safe Area" button. The ContentPage carries
// the safe-area knob — historically the iOSSpecific Page.UseSafeArea (the gallery page is named
// iOSSafeArea), reworked in current MAUI to the per-edge SafeAreaEdges replacement; the XAML seeds
// SafeAreaEdges="None" and OnButtonClicked sets this.SafeAreaEdges = SafeAreaEdges.None and disables the
// button. On UIKit the knob governs whether the page content lays out inside the device safe-area
// insets (notch / home indicator). Both knobs are honored at measure/arrange by content_page on the
// Apple backends and stored-inert headless, so this page renders the label + button AND exercises the
// config calls: the button flips safe area off and disables itself, mirroring OnButtonClicked.
//
// Code-first, owns its whole element tree (the value_controls_page / pickers_page pattern). Public
// page() hands back the content_page; the generic mount (app_host.hpp) attaches every owned view's
// handler and hosts the tree.

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/page.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/safe_area_edges.hpp"

namespace maui::samples
{
    class ios_safe_area_page
    {
    public:
        ios_safe_area_page()
        {
            namespace ios_page = maui::controls::platform_configuration::ios_specific::page;

            page_.set_title("Safe Area");

            // The Lorem-ipsum body Label (verbatim from the XAML so the page measures with real text the
            // safe-area inset visibly clips against on the Apple backends).
            lorem_.set_text(
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quis enim redargueret? At modo "
                "dixeras nihil in istis rebus esse, quod interesset. Et quidem, inquit, vehementer errat; "
                "Semper enim ex eo, quod maximas partes continet latissimeque funditur, tota res appellatur. "
                "Equidem, sed audistine modo de Carneade? Duo Reges: constructio interrete.");

            // SafeAreaEdges="None" — seed the per-edge safe-area knob (delegates to the content_page's
            // bindable property<safe_area_edges>). Honored by content_page measure/arrange on Apple
            // backends; the None default means the body extends under the device insets.
            ios_page::set_safe_area_edges(page_.on<maui::controls::platform_configuration::ios>(),
                                          maui::core::safe_area_edges::none());

            // Button Text="Disable Use Safe Area" / Clicked="OnButtonClicked".
            disable_button_.set_text("Disable Use Safe Area");
            disable_button_.clicked.connect([this] { on_disable_clicked(); });

            stack_.add(lorem_);
            stack_.add(disable_button_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& body()
        {
            return lorem_;
        }
        [[nodiscard]] maui::controls::button& disable_button()
        {
            return disable_button_;
        }

    private:
        void on_disable_clicked()
        {
            namespace ios_page = maui::controls::platform_configuration::ios_specific::page;
            const auto cfg = page_.on<maui::controls::platform_configuration::ios>();
            // OnButtonClicked: this.SafeAreaEdges = SafeAreaEdges.None — turn the safe area off.
            ios_page::set_safe_area_edges(cfg, maui::core::safe_area_edges::none());
            // Also exercise the legacy iOSSpecific Page.UseSafeArea knob this page historically demoed
            // (SafeAreaEdges::None == UseSafeArea=false: content ignores the safe-area insets). STORED on
            // headless; flowed through content_page measure/arrange on the Apple backends.
            ios_page::set_use_safe_area(cfg, false);
            // (sender as Button)!.IsEnabled = false — disable the button after the one-shot action.
            disable_button_.set_is_enabled(false);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label lorem_;
        maui::controls::button disable_button_;
    };
} // namespace maui::samples
