// Tests for the platform-configuration mechanism (W2-24) + the *Specific knob sets, on every backend
// (backend-agnostic — only the element store and the free-function knob surface are exercised).
//
// The five mechanism tests are ported from src/Controls/tests/Core.UnitTests/PlatformSpecificsTests.cs
// (the vendor knobs are test-local free functions over the same public store, exactly as C#'s
// ImAVendor namespaces define their own attached BindableProperties). The per-namespace storage
// round-trips are derived from the C# attached-property declarations (defaults / validation /
// propertyChanged quirks) in src/Controls/src/Core/PlatformConfiguration/** — C# ships no further
// unit tests for them.

#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/date_picker.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/element.hpp" // the shared store base the vendor knobs + i_*_specifics faces touch
#include "maui/controls/entry.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/image_button.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/platform_configuration/android_specific/app_compat/application.hpp"
#include "maui/controls/platform_configuration/android_specific/app_compat/navigation_page.hpp"
#include "maui/controls/platform_configuration/android_specific/application.hpp"
#include "maui/controls/platform_configuration/android_specific/button.hpp"
#include "maui/controls/platform_configuration/android_specific/entry.hpp"
#include "maui/controls/platform_configuration/android_specific/image_button.hpp"
#include "maui/controls/platform_configuration/android_specific/tabbed_page.hpp"
#include "maui/controls/platform_configuration/android_specific/toolbar_placement.hpp"
#include "maui/controls/platform_configuration/android_specific/visual_element.hpp"
#include "maui/controls/platform_configuration/android_specific/web_view.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/gtk_specific/navigation_page.hpp"
#include "maui/controls/platform_configuration/gtk_specific/tab_position.hpp"
#include "maui/controls/platform_configuration/gtk_specific/tabbed_page.hpp"
#include "maui/controls/platform_configuration/ios_specific/application.hpp"
#include "maui/controls/platform_configuration/ios_specific/blur_effect_style.hpp"
#include "maui/controls/platform_configuration/ios_specific/date_picker.hpp"
#include "maui/controls/platform_configuration/ios_specific/entry.hpp"
#include "maui/controls/platform_configuration/ios_specific/flyout_page.hpp"
#include "maui/controls/platform_configuration/ios_specific/large_title_display_mode.hpp"
#include "maui/controls/platform_configuration/ios_specific/navigation_page.hpp"
#include "maui/controls/platform_configuration/ios_specific/page.hpp"
#include "maui/controls/platform_configuration/ios_specific/picker.hpp"
#include "maui/controls/platform_configuration/ios_specific/scroll_view.hpp"
#include "maui/controls/platform_configuration/ios_specific/search_bar.hpp"
#include "maui/controls/platform_configuration/ios_specific/slider.hpp"
#include "maui/controls/platform_configuration/ios_specific/status_bar_hidden_mode.hpp"
#include "maui/controls/platform_configuration/ios_specific/status_bar_text_color_mode.hpp"
#include "maui/controls/platform_configuration/ios_specific/tabbed_page.hpp"
#include "maui/controls/platform_configuration/ios_specific/time_picker.hpp"
#include "maui/controls/platform_configuration/ios_specific/translucency_mode.hpp"
#include "maui/controls/platform_configuration/ios_specific/ui_modal_presentation_style.hpp"
#include "maui/controls/platform_configuration/ios_specific/ui_search_bar_style.hpp"
#include "maui/controls/platform_configuration/ios_specific/ui_status_bar_animation.hpp"
#include "maui/controls/platform_configuration/ios_specific/update_mode.hpp"
#include "maui/controls/platform_configuration/ios_specific/visual_element.hpp"
#include "maui/controls/platform_configuration/macos_specific/navigation_page.hpp"
#include "maui/controls/platform_configuration/macos_specific/navigation_transition_style.hpp"
#include "maui/controls/platform_configuration/macos_specific/page.hpp"
#include "maui/controls/platform_configuration/macos_specific/tabbed_page.hpp"
#include "maui/controls/platform_configuration/macos_specific/tabs_style.hpp"
#include "maui/controls/platform_configuration/tizen_specific/application.hpp"
#include "maui/controls/platform_configuration/tizen_specific/entry.hpp"
#include "maui/controls/platform_configuration/tizen_specific/focus_direction.hpp"
#include "maui/controls/platform_configuration/tizen_specific/font_weight.hpp"
#include "maui/controls/platform_configuration/tizen_specific/image.hpp"
#include "maui/controls/platform_configuration/tizen_specific/label.hpp"
#include "maui/controls/platform_configuration/tizen_specific/navigation_page.hpp"
#include "maui/controls/platform_configuration/tizen_specific/page.hpp"
#include "maui/controls/platform_configuration/tizen_specific/progress_bar.hpp"
#include "maui/controls/platform_configuration/tizen_specific/scroll_view.hpp"
#include "maui/controls/platform_configuration/tizen_specific/style_values.hpp"
#include "maui/controls/platform_configuration/tizen_specific/switch.hpp"
#include "maui/controls/platform_configuration/tizen_specific/visual_element.hpp"
#include "maui/controls/platform_configuration/windows_specific/access_key_placement.hpp"
#include "maui/controls/platform_configuration/windows_specific/application.hpp"
#include "maui/controls/platform_configuration/windows_specific/collapse_style.hpp"
#include "maui/controls/platform_configuration/windows_specific/flyout_page.hpp"
#include "maui/controls/platform_configuration/windows_specific/input_view.hpp"
#include "maui/controls/platform_configuration/windows_specific/label.hpp"
#include "maui/controls/platform_configuration/windows_specific/page.hpp"
#include "maui/controls/platform_configuration/windows_specific/search_bar.hpp"
#include "maui/controls/platform_configuration/windows_specific/tabbed_page.hpp"
#include "maui/controls/platform_configuration/windows_specific/toolbar_placement.hpp"
#include "maui/controls/platform_configuration/windows_specific/visual_element.hpp"
#include "maui/controls/platform_configuration/windows_specific/web_view.hpp"
#include "maui/controls/platform_configuration/windows_specific/web_view_execution_mode.hpp"
#include "maui/controls/progress_bar.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/web_view.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/i_ios_page_specifics.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    namespace pc = maui::controls::platform_configuration;
    using maui::controls::element;

    // ---- the C# test's ImAVendor.Forms.PlatformConfiguration.iOS.FlyoutPage vendor knob, ported as a
    // test-local knob over the SAME public store (proving the mechanism is open, like C# attached BPs) ----
    namespace vendor
    {
        constexpr std::string_view foo_key = "vendor.ios.FlyoutPage.VendorFoo";

        bool get_vendor_foo(const element& target)
        {
            return target.platform_spec<bool>(foo_key, true); // C# default true
        }
        void set_vendor_foo(element& target, bool value)
        {
            target.set_platform_spec(foo_key, value);
        }
        template <class TElement> bool get_vendor_foo(pc::config<pc::ios, TElement> cfg)
        {
            return get_vendor_foo(cfg.element());
        }
        template <class TElement>
        pc::config<pc::ios, TElement> set_vendor_foo(pc::config<pc::ios, TElement> cfg, bool value)
        {
            set_vendor_foo(cfg.element(), value);
            return cfg;
        }
    } // namespace vendor

    // ---- the C# test's mock AndroidSpecific.FlyoutPage knob set (SomeAndroidThing etc.) ----
    namespace mock_android
    {
        constexpr std::string_view some_thing_key = "android.FlyoutPage.SomeAndroidThing";
        constexpr std::string_view some_other_thing_key = "android.FlyoutPage.SomeOtherAndroidThing";

        template <class TElement> int get_some_android_thing(pc::config<pc::android, TElement> cfg)
        {
            return cfg.element().template platform_spec<int>(some_thing_key, 1); // C# default 1
        }
        template <class TElement>
        pc::config<pc::android, TElement> set_some_android_thing(pc::config<pc::android, TElement> cfg, int value)
        {
            cfg.element().set_platform_spec(some_thing_key, value);
            return cfg;
        }
        template <class TElement> int get_some_other_android_thing(pc::config<pc::android, TElement> cfg)
        {
            return cfg.element().template platform_spec<int>(some_other_thing_key, 1);
        }
        template <class TElement>
        pc::config<pc::android, TElement> set_some_other_android_thing(pc::config<pc::android, TElement> cfg, int value)
        {
            cfg.element().set_platform_spec(some_other_thing_key, value);
            return cfg;
        }
        template <class TElement>
        pc::config<pc::android, TElement> use_tablet_defaults(pc::config<pc::android, TElement> cfg)
        {
            set_some_android_thing(cfg, 10);
            set_some_other_android_thing(cfg, 45);
            return cfg;
        }
        template <class TElement>
        pc::config<pc::android, TElement> use_phablet_defaults(pc::config<pc::android, TElement> cfg)
        {
            set_some_android_thing(cfg, 8);
            set_some_other_android_thing(cfg, 40);
            return cfg;
        }
    } // namespace mock_android

    // ================= PlatformSpecificsTests.cs (the oracle's five mechanism tests) =================

    TEST(platform_configuration, vendor_platform_property)
    {
        maui::controls::flyout_page x;

        EXPECT_TRUE(vendor::get_vendor_foo(x.on<pc::ios>()));

        vendor::set_vendor_foo(x.on<pc::ios>(), false);

        EXPECT_FALSE(vendor::get_vendor_foo(x.on<pc::ios>()));
    }

    TEST(platform_configuration, consume_vendor_setting)
    {
        maui::controls::flyout_page x;
        vendor::set_vendor_foo(x.on<pc::ios>(), false);

        EXPECT_FALSE(vendor::get_vendor_foo(x.on<pc::ios>()));
    }

    TEST(platform_configuration, properties)
    {
        maui::controls::flyout_page x;
        mock_android::set_some_android_thing(x.on<pc::android>(), 42);

        EXPECT_EQ(mock_android::get_some_android_thing(x.on<pc::android>()), 42);
    }

    TEST(platform_configuration, convenience_configuration)
    {
        maui::controls::flyout_page x;

        mock_android::use_tablet_defaults(x.on<pc::android>());

        EXPECT_EQ(mock_android::get_some_android_thing(x.on<pc::android>()), 10);
        EXPECT_EQ(mock_android::get_some_other_android_thing(x.on<pc::android>()), 45);

        mock_android::use_phablet_defaults(x.on<pc::android>());

        EXPECT_EQ(mock_android::get_some_android_thing(x.on<pc::android>()), 8);
        EXPECT_EQ(mock_android::get_some_other_android_thing(x.on<pc::android>()), 40);
    }

    TEST(platform_configuration, navigation_page_ios_configuration)
    {
        maui::controls::navigation_page x;

        pc::ios_specific::navigation_page::set_is_navigation_bar_translucent(x.on<pc::ios>(), true);

        EXPECT_TRUE(pc::ios_specific::navigation_page::is_navigation_bar_translucent(x.on<pc::ios>()));
    }

    // ================= the mechanism itself (derived from BindableObject.SetValue semantics) ==========

    TEST(platform_configuration, on_returns_statically_typed_config)
    {
        maui::controls::entry control;
        auto cfg = control.on<pc::ios>();
        static_assert(std::is_same_v<decltype(cfg), pc::config<pc::ios, maui::controls::entry>>);
        EXPECT_EQ(&cfg.element(), &control);
    }

    TEST(platform_configuration, set_raises_property_changed_with_namespaced_key_once)
    {
        maui::controls::content_page page;
        std::vector<std::string> changed;
        page.property_changed.connect([&changed](std::string_view name) { changed.emplace_back(name); });

        pc::ios_specific::page::set_use_safe_area(page, true);
        ASSERT_EQ(changed.size(), 1U);
        EXPECT_EQ(changed[0], "ios.Page.UseSafeArea");

        // C# SetValue with an equal value is silent (SetValueActual's equality short-circuit).
        pc::ios_specific::page::set_use_safe_area(page, true);
        EXPECT_EQ(changed.size(), 1U);

        pc::ios_specific::page::set_use_safe_area(page, false);
        EXPECT_EQ(changed.size(), 2U);
    }

    TEST(platform_configuration, has_platform_spec_is_the_is_set_probe)
    {
        maui::controls::entry control;
        EXPECT_FALSE(control.has_platform_spec(pc::ios_specific::entry::cursor_color_key));
        pc::ios_specific::entry::set_cursor_color(control, maui::graphics::colors::red);
        EXPECT_TRUE(control.has_platform_spec(pc::ios_specific::entry::cursor_color_key));
    }

    // ================= iOSSpecific storage round-trips (defaults from the C# descriptors) =============

    TEST(platform_configuration_ios, application_knobs)
    {
        namespace k = pc::ios_specific::application;
        maui::controls::application app;
        EXPECT_FALSE(k::get_pan_gesture_recognizer_should_recognize_simultaneously(app));
        EXPECT_FALSE(k::get_handle_control_updates_on_main_thread(app));
        EXPECT_TRUE(k::get_enable_accessibility_scaling_for_named_font_sizes(app));

        k::set_pan_gesture_recognizer_should_recognize_simultaneously(app.on<pc::ios>(), true);
        k::set_handle_control_updates_on_main_thread(app.on<pc::ios>(), true);
        k::set_enable_accessibility_scaling_for_named_font_sizes(app.on<pc::ios>(), false);
        EXPECT_TRUE(k::get_pan_gesture_recognizer_should_recognize_simultaneously(app.on<pc::ios>()));
        EXPECT_TRUE(k::get_handle_control_updates_on_main_thread(app.on<pc::ios>()));
        EXPECT_FALSE(k::get_enable_accessibility_scaling_for_named_font_sizes(app.on<pc::ios>()));
    }

    TEST(platform_configuration_ios, picker_family_update_mode)
    {
        maui::controls::date_picker date_control;
        maui::controls::picker picker_control;
        maui::controls::time_picker time_control;
        using pc::ios_specific::update_mode;

        EXPECT_EQ(pc::ios_specific::date_picker::get_update_mode(date_control), update_mode::immediately);
        EXPECT_EQ(pc::ios_specific::picker::get_update_mode(picker_control), update_mode::immediately);
        EXPECT_EQ(pc::ios_specific::time_picker::get_update_mode(time_control), update_mode::immediately);

        pc::ios_specific::date_picker::set_update_mode(date_control.on<pc::ios>(), update_mode::when_finished);
        pc::ios_specific::picker::set_update_mode(picker_control.on<pc::ios>(), update_mode::when_finished);
        pc::ios_specific::time_picker::set_update_mode(time_control.on<pc::ios>(), update_mode::when_finished);
        EXPECT_EQ(pc::ios_specific::date_picker::update_mode(date_control.on<pc::ios>()), update_mode::when_finished);
        EXPECT_EQ(pc::ios_specific::picker::update_mode(picker_control.on<pc::ios>()), update_mode::when_finished);
        EXPECT_EQ(pc::ios_specific::time_picker::update_mode(time_control.on<pc::ios>()), update_mode::when_finished);
    }

    TEST(platform_configuration_ios, entry_knobs)
    {
        namespace k = pc::ios_specific::entry;
        maui::controls::entry control;
        EXPECT_FALSE(k::get_adjusts_font_size_to_fit_width(control));
        EXPECT_EQ(k::get_cursor_color(control), std::nullopt);

        k::enable_adjusts_font_size_to_fit_width(control.on<pc::ios>());
        EXPECT_TRUE(k::adjusts_font_size_to_fit_width(control.on<pc::ios>()));
        k::disable_adjusts_font_size_to_fit_width(control.on<pc::ios>());
        EXPECT_FALSE(k::adjusts_font_size_to_fit_width(control.on<pc::ios>()));

        k::set_cursor_color(control.on<pc::ios>(), maui::graphics::colors::lime);
        EXPECT_EQ(k::get_cursor_color(control.on<pc::ios>()), maui::graphics::colors::lime);
    }

    TEST(platform_configuration_ios, flyout_page_apply_shadow)
    {
        maui::controls::flyout_page page;
        EXPECT_FALSE(pc::ios_specific::flyout_page::get_apply_shadow(page));
        pc::ios_specific::flyout_page::set_apply_shadow(page.on<pc::ios>(), true);
        EXPECT_TRUE(pc::ios_specific::flyout_page::get_apply_shadow(page.on<pc::ios>()));
    }

    TEST(platform_configuration_ios, navigation_page_knobs)
    {
        namespace k = pc::ios_specific::navigation_page;
        maui::controls::navigation_page page;
        EXPECT_FALSE(k::get_is_navigation_bar_translucent(page));
        EXPECT_EQ(k::get_status_bar_text_color_mode(page),
                  pc::ios_specific::status_bar_text_color_mode::match_navigation_bar_text_luminosity);
        EXPECT_FALSE(k::get_prefers_large_titles(page));
        EXPECT_FALSE(k::get_hide_navigation_bar_separator(page));

        k::enable_translucent_navigation_bar(page.on<pc::ios>());
        EXPECT_TRUE(k::is_navigation_bar_translucent(page.on<pc::ios>()));
        k::disable_translucent_navigation_bar(page.on<pc::ios>());
        EXPECT_FALSE(k::is_navigation_bar_translucent(page.on<pc::ios>()));

        k::set_status_bar_text_color_mode(page.on<pc::ios>(),
                                          pc::ios_specific::status_bar_text_color_mode::do_not_adjust);
        EXPECT_EQ(k::get_status_bar_text_color_mode(page.on<pc::ios>()),
                  pc::ios_specific::status_bar_text_color_mode::do_not_adjust);
        k::set_prefers_large_titles(page.on<pc::ios>(), true);
        EXPECT_TRUE(k::prefers_large_titles(page.on<pc::ios>()));
        k::set_hide_navigation_bar_separator(page.on<pc::ios>(), true);
        EXPECT_TRUE(k::hide_navigation_bar_separator(page.on<pc::ios>()));
    }

    TEST(platform_configuration_ios, page_knobs)
    {
        namespace k = pc::ios_specific::page;
        maui::controls::content_page page;
        EXPECT_EQ(k::get_prefers_status_bar_hidden(page), pc::ios_specific::status_bar_hidden_mode::default_mode);
        EXPECT_EQ(k::get_preferred_status_bar_update_animation(page), pc::ios_specific::ui_status_bar_animation::none);
        EXPECT_FALSE(k::get_use_safe_area(page));
        EXPECT_EQ(k::get_large_title_display(page), pc::ios_specific::large_title_display_mode::automatic);
        EXPECT_EQ(k::get_safe_area_insets(page), maui::core::thickness{});
        EXPECT_EQ(k::get_modal_presentation_style(page), pc::ios_specific::ui_modal_presentation_style::full_screen);
        EXPECT_EQ(k::get_popover_source_view(page), nullptr);
        EXPECT_EQ(k::get_popover_rect(page), maui::graphics::rect{});
        EXPECT_FALSE(k::get_prefers_home_indicator_auto_hidden(page));

        k::set_prefers_status_bar_hidden(page.on<pc::ios>(), pc::ios_specific::status_bar_hidden_mode::true_mode);
        EXPECT_EQ(k::prefers_status_bar_hidden(page.on<pc::ios>()),
                  pc::ios_specific::status_bar_hidden_mode::true_mode);
        k::set_preferred_status_bar_update_animation(page.on<pc::ios>(),
                                                     pc::ios_specific::ui_status_bar_animation::slide);
        EXPECT_EQ(k::preferred_status_bar_update_animation(page.on<pc::ios>()),
                  pc::ios_specific::ui_status_bar_animation::slide);
        k::set_use_safe_area(page.on<pc::ios>(), true);
        EXPECT_TRUE(k::using_safe_area(page.on<pc::ios>()));
        k::set_large_title_display(page.on<pc::ios>(), pc::ios_specific::large_title_display_mode::never);
        EXPECT_EQ(k::large_title_display(page.on<pc::ios>()), pc::ios_specific::large_title_display_mode::never);
        k::set_safe_area_insets(page.on<pc::ios>(), maui::core::thickness{1, 2, 3, 4});
        EXPECT_EQ(k::safe_area_insets(page.on<pc::ios>()), (maui::core::thickness{1, 2, 3, 4}));
        k::set_modal_presentation_style(page.on<pc::ios>(), pc::ios_specific::ui_modal_presentation_style::popover);
        EXPECT_EQ(k::modal_presentation_style(page.on<pc::ios>()),
                  pc::ios_specific::ui_modal_presentation_style::popover);
        maui::controls::button source;
        k::set_modal_popover_view(page.on<pc::ios>(), &source);
        EXPECT_EQ(k::modal_popover_source_view(page.on<pc::ios>()), &source);
        k::set_modal_popover_rect(page.on<pc::ios>(), maui::graphics::rect{1, 2, 3, 4});
        EXPECT_EQ(k::modal_popover_rect(page.on<pc::ios>()), (maui::graphics::rect{1, 2, 3, 4}));
        k::set_prefers_home_indicator_auto_hidden(page.on<pc::ios>(), true);
        EXPECT_TRUE(k::prefers_home_indicator_auto_hidden(page.on<pc::ios>()));
    }

    // C# IPlatformElementConfiguration is covariant: the Page extensions apply to a NavigationPage's
    // config too. The port's page_element constraint reproduces that admission.
    TEST(platform_configuration_ios, page_knobs_apply_to_derived_page_types)
    {
        maui::controls::navigation_page nav;
        pc::ios_specific::page::set_prefers_home_indicator_auto_hidden(nav.on<pc::ios>(), true);
        EXPECT_TRUE(pc::ios_specific::page::prefers_home_indicator_auto_hidden(nav.on<pc::ios>()));
    }

    TEST(platform_configuration_ios, scroll_view_slider_search_bar_tabbed_page)
    {
        maui::controls::scroll_view scroll;
        EXPECT_TRUE(pc::ios_specific::scroll_view::get_should_delay_content_touches(scroll));
        pc::ios_specific::scroll_view::set_should_delay_content_touches(scroll.on<pc::ios>(), false);
        EXPECT_FALSE(pc::ios_specific::scroll_view::should_delay_content_touches(scroll.on<pc::ios>()));

        maui::controls::slider slider_control;
        EXPECT_FALSE(pc::ios_specific::slider::get_update_on_tap(slider_control));
        pc::ios_specific::slider::set_update_on_tap(slider_control.on<pc::ios>(), true);
        EXPECT_TRUE(pc::ios_specific::slider::get_update_on_tap(slider_control.on<pc::ios>()));

        maui::controls::search_bar bar;
        EXPECT_EQ(pc::ios_specific::search_bar::get_search_bar_style(bar),
                  pc::ios_specific::ui_search_bar_style::default_style);
        pc::ios_specific::search_bar::set_search_bar_style(bar.on<pc::ios>(),
                                                           pc::ios_specific::ui_search_bar_style::minimal);
        EXPECT_EQ(pc::ios_specific::search_bar::get_search_bar_style(bar.on<pc::ios>()),
                  pc::ios_specific::ui_search_bar_style::minimal);

        maui::controls::tabbed_page tabs;
        EXPECT_EQ(pc::ios_specific::tabbed_page::get_translucency_mode(tabs),
                  pc::ios_specific::translucency_mode::default_mode);
        pc::ios_specific::tabbed_page::set_translucency_mode(tabs.on<pc::ios>(),
                                                             pc::ios_specific::translucency_mode::opaque);
        EXPECT_EQ(pc::ios_specific::tabbed_page::get_translucency_mode(tabs.on<pc::ios>()),
                  pc::ios_specific::translucency_mode::opaque);
    }

    TEST(platform_configuration_ios, visual_element_knobs)
    {
        namespace k = pc::ios_specific::visual_element;
        maui::controls::button control;
        EXPECT_EQ(k::get_blur_effect(control), pc::ios_specific::blur_effect_style::none);
        EXPECT_FALSE(k::get_is_shadow_enabled(control));
        EXPECT_EQ(k::get_shadow_color(control), std::nullopt);
        EXPECT_EQ(k::get_shadow_radius(control), 10.0);
        EXPECT_EQ(k::get_shadow_offset(control), maui::graphics::size{});
        EXPECT_EQ(k::get_shadow_opacity(control), 0.5);
        EXPECT_TRUE(k::get_is_legacy_color_mode_enabled(control));
        EXPECT_FALSE(k::get_can_become_first_responder(control));

        k::use_blur_effect(control.on<pc::ios>(), pc::ios_specific::blur_effect_style::dark);
        EXPECT_EQ(k::get_blur_effect(control.on<pc::ios>()), pc::ios_specific::blur_effect_style::dark);
        k::set_is_shadow_enabled(control.on<pc::ios>(), true);
        EXPECT_TRUE(k::get_is_shadow_enabled(control.on<pc::ios>()));
        k::set_shadow_color(control.on<pc::ios>(), maui::graphics::colors::blue);
        EXPECT_EQ(k::get_shadow_color(control.on<pc::ios>()), maui::graphics::colors::blue);
        k::set_shadow_radius(control.on<pc::ios>(), 4.0);
        EXPECT_EQ(k::get_shadow_radius(control.on<pc::ios>()), 4.0);
        k::set_shadow_offset(control.on<pc::ios>(), maui::graphics::size{2, 3});
        EXPECT_EQ(k::get_shadow_offset(control.on<pc::ios>()), (maui::graphics::size{2, 3}));
        k::set_shadow_opacity(control.on<pc::ios>(), 0.25);
        EXPECT_EQ(k::get_shadow_opacity(control.on<pc::ios>()), 0.25);
        k::set_is_legacy_color_mode_enabled(control.on<pc::ios>(), false);
        EXPECT_FALSE(k::get_is_legacy_color_mode_enabled(control.on<pc::ios>()));
        k::set_can_become_first_responder(control.on<pc::ios>(), true);
        EXPECT_TRUE(k::can_become_first_responder(control.on<pc::ios>()));
    }

    // ================= macOSSpecific (stored-inert; see the headers for the rationale) =================

    TEST(platform_configuration_macos, navigation_page_transition_styles)
    {
        namespace k = pc::macos_specific::navigation_page;
        using pc::macos_specific::navigation_transition_style;
        maui::controls::navigation_page page;
        EXPECT_EQ(k::get_navigation_transition_push_style(page), navigation_transition_style::slide_forward);
        EXPECT_EQ(k::get_navigation_transition_pop_style(page), navigation_transition_style::slide_backward);

        k::set_navigation_transition_style(page.on<pc::macos>(), navigation_transition_style::crossfade,
                                           navigation_transition_style::slide_up);
        EXPECT_EQ(k::get_navigation_transition_push_style(page.on<pc::macos>()),
                  navigation_transition_style::crossfade);
        EXPECT_EQ(k::get_navigation_transition_pop_style(page.on<pc::macos>()), navigation_transition_style::slide_up);
    }

    TEST(platform_configuration_macos, page_tab_order)
    {
        maui::controls::content_page page;
        maui::controls::button first;
        maui::controls::button second;
        EXPECT_TRUE(pc::macos_specific::page::get_tab_order(page).empty()); // C# default null

        pc::macos_specific::page::set_tab_order(page.on<pc::macos>(), {&first, &second});
        const auto order = pc::macos_specific::page::get_tab_order(page.on<pc::macos>());
        ASSERT_EQ(order.size(), 2U);
        EXPECT_EQ(order[0], &first);
        EXPECT_EQ(order[1], &second);
    }

    TEST(platform_configuration_macos, tabbed_page_tabs_style)
    {
        namespace k = pc::macos_specific::tabbed_page;
        using pc::macos_specific::tabs_style;
        maui::controls::tabbed_page tabs;
        EXPECT_EQ(k::get_tabs_style(tabs), tabs_style::default_style);

        k::hide_tabs(tabs.on<pc::macos>());
        EXPECT_EQ(k::get_tabs_style(tabs.on<pc::macos>()), tabs_style::hidden);
        k::show_tabs_on_navigation(tabs.on<pc::macos>());
        EXPECT_EQ(k::get_tabs_style(tabs.on<pc::macos>()), tabs_style::on_navigation);
        k::show_tabs(tabs.on<pc::macos>());
        EXPECT_EQ(k::get_tabs_style(tabs.on<pc::macos>()), tabs_style::default_style);
        k::set_show_tabs(tabs.on<pc::macos>(), tabs_style::icons);
        EXPECT_EQ(k::get_tabs_style(tabs.on<pc::macos>()), tabs_style::icons);
    }

    // ================= AndroidSpecific (stored-inert until the JNI fan-out) ===========================

    TEST(platform_configuration_android, application_knobs)
    {
        maui::controls::application app;
        using pc::android_specific::window_soft_input_mode_adjust;
        EXPECT_EQ(pc::android_specific::application::get_window_soft_input_mode_adjust(app),
                  window_soft_input_mode_adjust::pan);
        pc::android_specific::application::use_window_soft_input_mode_adjust(app.on<pc::android>(),
                                                                             window_soft_input_mode_adjust::resize);
        EXPECT_EQ(pc::android_specific::application::get_window_soft_input_mode_adjust(app.on<pc::android>()),
                  window_soft_input_mode_adjust::resize);

        namespace ac = pc::android_specific::app_compat::application;
        EXPECT_TRUE(ac::get_send_disappearing_event_on_pause(app));
        EXPECT_TRUE(ac::get_send_appearing_event_on_resume(app));
        EXPECT_FALSE(ac::get_should_preserve_keyboard_on_resume(app));
        ac::send_disappearing_event_on_pause(app.on<pc::android>(), false);
        ac::send_appearing_event_on_resume(app.on<pc::android>(), false);
        ac::should_preserve_keyboard_on_resume(app.on<pc::android>(), true);
        EXPECT_FALSE(ac::get_send_disappearing_event_on_pause(app.on<pc::android>()));
        EXPECT_FALSE(ac::get_send_appearing_event_on_resume(app.on<pc::android>()));
        EXPECT_TRUE(ac::get_should_preserve_keyboard_on_resume(app.on<pc::android>()));
    }

    TEST(platform_configuration_android, button_knobs)
    {
        namespace k = pc::android_specific::button;
        maui::controls::button control;
        EXPECT_FALSE(k::get_use_default_padding(control));
        EXPECT_FALSE(k::get_use_default_shadow(control));
        EXPECT_EQ(k::get_ripple_color(control), std::nullopt);

        k::set_use_default_padding(control.on<pc::android>(), true);
        k::set_use_default_shadow(control.on<pc::android>(), true);
        k::set_ripple_color(control.on<pc::android>(), maui::graphics::colors::red);
        EXPECT_TRUE(k::use_default_padding(control.on<pc::android>()));
        EXPECT_TRUE(k::use_default_shadow(control.on<pc::android>()));
        EXPECT_EQ(k::get_ripple_color(control.on<pc::android>()), maui::graphics::colors::red);
    }

    TEST(platform_configuration_android, entry_ime_options)
    {
        maui::controls::entry control;
        using pc::android_specific::ime_flags;
        EXPECT_EQ(pc::android_specific::entry::get_ime_options(control), ime_flags::default_flags);
        pc::android_specific::entry::set_ime_options(control.on<pc::android>(), ime_flags::search);
        EXPECT_EQ(pc::android_specific::entry::ime_options(control.on<pc::android>()), ime_flags::search);
    }

    TEST(platform_configuration_android, image_button_knobs)
    {
        namespace k = pc::android_specific::image_button;
        maui::controls::image_button control;
        EXPECT_FALSE(k::get_is_shadow_enabled(control));
        EXPECT_EQ(k::get_shadow_color(control), std::nullopt);
        EXPECT_EQ(k::get_shadow_radius(control), 10.0);
        EXPECT_EQ(k::get_shadow_offset(control), maui::graphics::size{});
        EXPECT_EQ(k::get_ripple_color(control), std::nullopt);

        k::set_is_shadow_enabled(control.on<pc::android>(), true);
        k::set_shadow_color(control.on<pc::android>(), maui::graphics::colors::black);
        k::set_shadow_radius(control.on<pc::android>(), 2.5);
        k::set_shadow_offset(control.on<pc::android>(), maui::graphics::size{1, 1});
        k::set_ripple_color(control.on<pc::android>(), maui::graphics::colors::lime);
        EXPECT_TRUE(k::get_is_shadow_enabled(control.on<pc::android>()));
        EXPECT_EQ(k::get_shadow_color(control.on<pc::android>()), maui::graphics::colors::black);
        EXPECT_EQ(k::get_shadow_radius(control.on<pc::android>()), 2.5);
        EXPECT_EQ(k::get_shadow_offset(control.on<pc::android>()), (maui::graphics::size{1, 1}));
        EXPECT_EQ(k::get_ripple_color(control.on<pc::android>()), maui::graphics::colors::lime);
    }

    TEST(platform_configuration_android, tabbed_page_knobs)
    {
        namespace k = pc::android_specific::tabbed_page;
        using pc::android_specific::toolbar_placement;
        maui::controls::tabbed_page tabs;
        EXPECT_TRUE(k::get_is_swipe_paging_enabled(tabs));
        EXPECT_TRUE(k::get_is_smooth_scroll_enabled(tabs));
        EXPECT_EQ(k::get_offscreen_page_limit(tabs), 3);
        EXPECT_EQ(k::get_toolbar_placement(tabs), toolbar_placement::top);
        EXPECT_EQ(k::get_max_item_count(tabs), std::numeric_limits<int>::max());

        k::disable_swipe_paging(tabs.on<pc::android>());
        EXPECT_FALSE(k::is_swipe_paging_enabled(tabs.on<pc::android>()));
        k::enable_swipe_paging(tabs.on<pc::android>());
        EXPECT_TRUE(k::is_swipe_paging_enabled(tabs.on<pc::android>()));
        k::disable_smooth_scroll(tabs.on<pc::android>());
        EXPECT_FALSE(k::is_smooth_scroll_enabled(tabs.on<pc::android>()));

        k::set_offscreen_page_limit(tabs.on<pc::android>(), 5);
        EXPECT_EQ(k::offscreen_page_limit(tabs.on<pc::android>()), 5);
        // C# validateValue rejects negatives (SetValue throws ArgumentException).
        EXPECT_THROW(k::set_offscreen_page_limit(tabs, -1), std::invalid_argument);

        k::set_toolbar_placement(tabs.on<pc::android>(), toolbar_placement::bottom);
        EXPECT_EQ(k::get_toolbar_placement(tabs.on<pc::android>()), toolbar_placement::bottom);
        EXPECT_EQ(k::get_max_item_count(tabs.on<pc::android>()), 5); // bottom placement caps at 5
    }

    TEST(platform_configuration_android, visual_element_and_web_view_knobs)
    {
        maui::controls::label control;
        EXPECT_EQ(pc::android_specific::visual_element::get_elevation(control), std::nullopt);
        EXPECT_TRUE(pc::android_specific::visual_element::get_is_legacy_color_mode_enabled(control));
        pc::android_specific::visual_element::set_elevation(control.on<pc::android>(), 4.0F);
        EXPECT_EQ(pc::android_specific::visual_element::get_elevation(control.on<pc::android>()), 4.0F);
        pc::android_specific::visual_element::set_is_legacy_color_mode_enabled(control.on<pc::android>(), false);
        EXPECT_FALSE(pc::android_specific::visual_element::get_is_legacy_color_mode_enabled(control.on<pc::android>()));

        namespace w = pc::android_specific::web_view;
        using pc::android_specific::mixed_content_handling;
        maui::controls::web_view web;
        EXPECT_EQ(w::get_mixed_content_mode(web), mixed_content_handling::never_allow);
        EXPECT_FALSE(w::get_enable_zoom_controls(web));
        EXPECT_TRUE(w::get_display_zoom_controls(web));
        EXPECT_TRUE(w::get_javascript_enabled(web));

        w::set_mixed_content_mode(web.on<pc::android>(), mixed_content_handling::always_allow);
        EXPECT_EQ(w::mixed_content_mode(web.on<pc::android>()), mixed_content_handling::always_allow);
        w::enable_zoom_controls(web.on<pc::android>(), true);
        EXPECT_TRUE(w::zoom_controls_enabled(web.on<pc::android>()));
        w::display_zoom_controls(web.on<pc::android>(), false);
        EXPECT_FALSE(w::zoom_controls_displayed(web.on<pc::android>()));
        w::javascript_enabled(web.on<pc::android>(), false);
        EXPECT_FALSE(w::is_javascript_enabled(web.on<pc::android>()));

        maui::controls::navigation_page nav;
        EXPECT_EQ(pc::android_specific::app_compat::navigation_page::get_bar_height(nav), 0);
        pc::android_specific::app_compat::navigation_page::set_bar_height(nav.on<pc::android>(), 56);
        EXPECT_EQ(pc::android_specific::app_compat::navigation_page::get_bar_height(nav.on<pc::android>()), 56);
    }

    // ================= WindowsSpecific (stored-inert; no Windows backend) =============================

    TEST(platform_configuration_windows, application_and_pages)
    {
        maui::controls::application app;
        EXPECT_EQ(pc::windows_specific::application::get_image_directory(app), "");
        pc::windows_specific::application::set_image_directory(app.on<pc::windows>(), "Assets");
        EXPECT_EQ(pc::windows_specific::application::get_image_directory(app.on<pc::windows>()), "Assets");

        namespace fp = pc::windows_specific::flyout_page;
        using pc::windows_specific::collapse_style;
        maui::controls::flyout_page flyout;
        EXPECT_EQ(fp::get_collapse_style(flyout), collapse_style::full);
        EXPECT_EQ(fp::get_collapsed_pane_width(flyout), 48.0);
        fp::use_partial_collapse(flyout.on<pc::windows>());
        EXPECT_EQ(fp::get_collapse_style(flyout.on<pc::windows>()), collapse_style::partial);
        fp::collapsed_pane_width(flyout.on<pc::windows>(), 100.0);
        EXPECT_EQ(fp::collapsed_pane_width(flyout.on<pc::windows>()), 100.0);
        EXPECT_THROW(fp::set_collapsed_pane_width(flyout, -1.0), std::invalid_argument);

        namespace pg = pc::windows_specific::page;
        using pc::windows_specific::toolbar_placement;
        maui::controls::content_page page;
        EXPECT_EQ(pg::get_toolbar_placement(page), toolbar_placement::default_placement);
        EXPECT_TRUE(pg::get_toolbar_dynamic_overflow_enabled(page));
        pg::set_toolbar_placement(page.on<pc::windows>(), toolbar_placement::bottom);
        EXPECT_EQ(pg::get_toolbar_placement(page.on<pc::windows>()), toolbar_placement::bottom);
        pg::set_toolbar_dynamic_overflow_enabled(page.on<pc::windows>(), false);
        EXPECT_FALSE(pg::get_toolbar_dynamic_overflow_enabled(page.on<pc::windows>()));
    }

    TEST(platform_configuration_windows, input_and_text_controls)
    {
        // InputView targets the declared trio; the Label twin is a DISTINCT key (distinct C# declaring types).
        maui::controls::entry entry_control;
        maui::controls::editor editor_control;
        namespace iv = pc::windows_specific::input_view;
        EXPECT_FALSE(iv::get_detect_reading_order_from_content(entry_control));
        iv::set_detect_reading_order_from_content(entry_control.on<pc::windows>(), true);
        iv::set_detect_reading_order_from_content(editor_control.on<pc::windows>(), true);
        EXPECT_TRUE(iv::get_detect_reading_order_from_content(entry_control.on<pc::windows>()));
        EXPECT_TRUE(iv::get_detect_reading_order_from_content(editor_control.on<pc::windows>()));

        maui::controls::label label_control;
        namespace lb = pc::windows_specific::label;
        EXPECT_FALSE(lb::get_detect_reading_order_from_content(label_control));
        lb::set_detect_reading_order_from_content(label_control.on<pc::windows>(), true);
        EXPECT_TRUE(lb::get_detect_reading_order_from_content(label_control.on<pc::windows>()));

        maui::controls::search_bar bar;
        namespace sb = pc::windows_specific::search_bar;
        EXPECT_FALSE(sb::get_is_spell_check_enabled(bar));
        sb::enable_spell_check(bar.on<pc::windows>());
        EXPECT_TRUE(sb::is_spell_check_enabled(bar.on<pc::windows>()));
        sb::disable_spell_check(bar.on<pc::windows>());
        EXPECT_FALSE(sb::is_spell_check_enabled(bar.on<pc::windows>()));
    }

    TEST(platform_configuration_windows, tabbed_page_visual_element_web_view)
    {
        namespace tp = pc::windows_specific::tabbed_page;
        maui::controls::tabbed_page tabs;
        EXPECT_TRUE(tp::get_header_icons_enabled(tabs));
        EXPECT_EQ(tp::get_header_icons_size(tabs), (maui::graphics::size{16, 16}));
        tp::disable_header_icons(tabs.on<pc::windows>());
        EXPECT_FALSE(tp::is_header_icons_enabled(tabs.on<pc::windows>()));
        tp::enable_header_icons(tabs.on<pc::windows>());
        EXPECT_TRUE(tp::is_header_icons_enabled(tabs.on<pc::windows>()));
        tp::set_header_icons_size(tabs.on<pc::windows>(), maui::graphics::size{24, 24});
        EXPECT_EQ(tp::get_header_icons_size(tabs.on<pc::windows>()), (maui::graphics::size{24, 24}));

        namespace ve = pc::windows_specific::visual_element;
        using pc::windows_specific::access_key_placement;
        maui::controls::button control;
        EXPECT_EQ(ve::get_access_key(control), "");
        EXPECT_EQ(ve::get_access_key_placement(control), access_key_placement::automatic);
        EXPECT_EQ(ve::get_access_key_horizontal_offset(control), 0.0);
        EXPECT_EQ(ve::get_access_key_vertical_offset(control), 0.0);
        EXPECT_TRUE(ve::get_is_legacy_color_mode_enabled(control));
        ve::set_access_key(control.on<pc::windows>(), "K");
        ve::set_access_key_placement(control.on<pc::windows>(), access_key_placement::top);
        ve::set_access_key_horizontal_offset(control.on<pc::windows>(), 2.0);
        ve::set_access_key_vertical_offset(control.on<pc::windows>(), 3.0);
        ve::set_is_legacy_color_mode_enabled(control.on<pc::windows>(), false);
        EXPECT_EQ(ve::get_access_key(control.on<pc::windows>()), "K");
        EXPECT_EQ(ve::get_access_key_placement(control.on<pc::windows>()), access_key_placement::top);
        EXPECT_EQ(ve::get_access_key_horizontal_offset(control.on<pc::windows>()), 2.0);
        EXPECT_EQ(ve::get_access_key_vertical_offset(control.on<pc::windows>()), 3.0);
        EXPECT_FALSE(ve::get_is_legacy_color_mode_enabled(control.on<pc::windows>()));

        namespace wv = pc::windows_specific::web_view;
        using pc::windows_specific::web_view_execution_mode;
        maui::controls::web_view web;
        EXPECT_FALSE(wv::get_is_java_script_alert_enabled(web));
        EXPECT_EQ(wv::get_execution_mode(web), web_view_execution_mode::same_thread);
        wv::set_is_java_script_alert_enabled(web.on<pc::windows>(), true);
        EXPECT_TRUE(wv::is_java_script_alert_enabled(web.on<pc::windows>()));
        wv::set_execution_mode(web.on<pc::windows>(), web_view_execution_mode::separate_process);
        EXPECT_EQ(wv::get_execution_mode(web.on<pc::windows>()), web_view_execution_mode::separate_process);
    }

    // ================= TizenSpecific (stored-inert; no Tizen backend) =================================

    TEST(platform_configuration_tizen, application_knobs)
    {
        namespace k = pc::tizen_specific::application;
        maui::controls::application app;
        maui::controls::button overlay;
        maui::controls::button active;
        EXPECT_TRUE(k::get_use_bezel_interaction(app));
        EXPECT_EQ(k::get_overlay_content(app), nullptr);
        EXPECT_EQ(k::get_active_bezel_interaction_element(app), nullptr);

        k::set_use_bezel_interaction(app.on<pc::tizen>(), false);
        k::set_overlay_content(app.on<pc::tizen>(), &overlay);
        k::set_active_bezel_interaction_element(app.on<pc::tizen>(), &active);
        EXPECT_FALSE(k::get_use_bezel_interaction(app.on<pc::tizen>()));
        EXPECT_EQ(k::get_overlay_content(app.on<pc::tizen>()), &overlay);
        EXPECT_EQ(k::get_active_bezel_interaction_element(app.on<pc::tizen>()), &active);
    }

    TEST(platform_configuration_tizen, text_and_image_knobs)
    {
        maui::controls::entry entry_control;
        EXPECT_EQ(pc::tizen_specific::entry::get_font_weight(entry_control), pc::tizen_specific::font_weight::none);
        pc::tizen_specific::entry::set_font_weight(entry_control.on<pc::tizen>(),
                                                   std::string{pc::tizen_specific::font_weight::bold});
        EXPECT_EQ(pc::tizen_specific::entry::get_font_weight(entry_control.on<pc::tizen>()), "Bold");

        maui::controls::label label_control;
        EXPECT_EQ(pc::tizen_specific::label::get_font_weight(label_control), pc::tizen_specific::font_weight::none);
        pc::tizen_specific::label::set_font_weight(label_control.on<pc::tizen>(),
                                                   std::string{pc::tizen_specific::font_weight::medium});
        EXPECT_EQ(pc::tizen_specific::label::get_font_weight(label_control.on<pc::tizen>()), "Medium");

        maui::controls::image image_control;
        EXPECT_EQ(pc::tizen_specific::image::get_blend_color(image_control), std::nullopt);
        EXPECT_EQ(pc::tizen_specific::image::get_file(image_control), "");
        pc::tizen_specific::image::set_blend_color(image_control.on<pc::tizen>(), maui::graphics::colors::red);
        pc::tizen_specific::image::set_file(image_control.on<pc::tizen>(), "icon.png");
        EXPECT_EQ(pc::tizen_specific::image::get_blend_color(image_control.on<pc::tizen>()),
                  maui::graphics::colors::red);
        EXPECT_EQ(pc::tizen_specific::image::get_file(image_control.on<pc::tizen>()), "icon.png");
    }

    TEST(platform_configuration_tizen, page_and_navigation_knobs)
    {
        maui::controls::navigation_page nav;
        EXPECT_FALSE(pc::tizen_specific::navigation_page::get_has_bread_crumbs_bar(nav));
        pc::tizen_specific::navigation_page::set_has_bread_crumbs_bar(nav.on<pc::tizen>(), true);
        EXPECT_TRUE(pc::tizen_specific::navigation_page::has_bread_crumbs_bar(nav.on<pc::tizen>()));

        maui::controls::content_page page;
        EXPECT_EQ(pc::tizen_specific::page::get_bread_crumb(page), "");
        pc::tizen_specific::page::set_bread_crumb(page.on<pc::tizen>(), "Home");
        EXPECT_EQ(pc::tizen_specific::page::get_bread_crumb(page.on<pc::tizen>()), "Home");
    }

    TEST(platform_configuration_tizen, progress_bar_pulsing_requires_pending_style)
    {
        namespace k = pc::tizen_specific::progress_bar;
        maui::controls::progress_bar control;
        EXPECT_FALSE(k::get_pulsing_status(control));

        // C# SetPulsingStatus is a silent no-op unless the Tizen ThemeStyle is ProgressBarStyle.Pending.
        k::set_pulsing_status(control, true);
        EXPECT_FALSE(k::get_pulsing_status(control));

        pc::tizen_specific::visual_element::set_style(control,
                                                      std::string{pc::tizen_specific::progress_bar_style::pending});
        k::set_pulsing_status(control.on<pc::tizen>(), true);
        EXPECT_TRUE(k::get_pulsing_status(control.on<pc::tizen>()));
    }

    TEST(platform_configuration_tizen, scroll_steps_coerce_negatives)
    {
        namespace k = pc::tizen_specific::scroll_view;
        maui::controls::scroll_view scroll;
        EXPECT_EQ(k::get_vertical_scroll_step(scroll), -1);
        EXPECT_EQ(k::get_horizontal_scroll_step(scroll), -1);

        k::set_vertical_scroll_step(scroll.on<pc::tizen>(), 24);
        k::set_horizontal_scroll_step(scroll.on<pc::tizen>(), 12);
        EXPECT_EQ(k::get_vertical_scroll_step(scroll.on<pc::tizen>()), 24);
        EXPECT_EQ(k::get_horizontal_scroll_step(scroll.on<pc::tizen>()), 12);

        // C# coerceValue: negatives coerce to -1.
        k::set_vertical_scroll_step(scroll.on<pc::tizen>(), -5);
        EXPECT_EQ(k::get_vertical_scroll_step(scroll.on<pc::tizen>()), -1);
    }

    TEST(platform_configuration_tizen, switch_color)
    {
        maui::controls::toggle_switch control;
        EXPECT_EQ(pc::tizen_specific::switch_control::get_color(control), std::nullopt);
        pc::tizen_specific::switch_control::set_color(control.on<pc::tizen>(), maui::graphics::colors::lime);
        EXPECT_EQ(pc::tizen_specific::switch_control::get_color(control.on<pc::tizen>()), maui::graphics::colors::lime);
    }

    TEST(platform_configuration_tizen, visual_element_knobs)
    {
        namespace k = pc::tizen_specific::visual_element;
        maui::controls::button control;
        maui::controls::button up;
        maui::controls::button down;
        EXPECT_EQ(k::get_style(control), "");
        EXPECT_EQ(k::is_focus_allowed(control), std::nullopt);
        EXPECT_EQ(k::get_next_focus_direction(control), pc::tizen_specific::focus_direction::none);
        EXPECT_EQ(k::get_next_focus_up_view(control), nullptr);
        EXPECT_EQ(k::get_tool_tip(control), "");

        k::set_style(control.on<pc::tizen>(), std::string{pc::tizen_specific::button_style::circle});
        EXPECT_EQ(k::get_style(control.on<pc::tizen>()), "circle");
        k::set_focus_allowed(control.on<pc::tizen>(), true);
        EXPECT_EQ(k::is_focus_allowed(control.on<pc::tizen>()), true);
        k::set_next_focus_up_view(control.on<pc::tizen>(), &up);
        k::set_next_focus_down_view(control.on<pc::tizen>(), &down);
        EXPECT_EQ(k::get_next_focus_up_view(control.on<pc::tizen>()), &up);
        EXPECT_EQ(k::get_next_focus_down_view(control.on<pc::tizen>()), &down);
        k::set_tool_tip(control.on<pc::tizen>(), "tip");
        EXPECT_EQ(k::get_tool_tip(control.on<pc::tizen>()), "tip");
    }

    TEST(platform_configuration_tizen, next_focus_direction_is_one_shot)
    {
        // C#'s NextFocusDirection propertyChanged immediately resets the stored value to None: the set
        // raises the change (the renderer's focus-move trigger) but a read always observes "None".
        namespace k = pc::tizen_specific::visual_element;
        maui::controls::button control;
        std::vector<std::string> changed;
        control.property_changed.connect([&changed](std::string_view name) {
            if (name == k::next_focus_direction_key)
            {
                changed.emplace_back(name);
            }
        });

        k::move_focus_up(control.on<pc::tizen>());
        EXPECT_EQ(k::get_next_focus_direction(control), pc::tizen_specific::focus_direction::none);
        EXPECT_EQ(changed.size(), 2U); // the direction change + the reset, like C#'s SetValue pair
    }

    // ================= the WIRED-REAL subset's cross-platform seams (W2-24) ===========================
    // Every backend maintains these mirrors (the iOS twin additionally drives the real UIKit objects —
    // see tests/controls/platform_configuration_ios_tests.mm), so the seam is observable everywhere.

    TEST(platform_configuration_wired, entry_cursor_color_reaches_platform_mirror)
    {
        maui::controls::entry control;
        auto handler = std::make_shared<maui::core::entry_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // TextExtensions.UpdateCursorColor's IsSet guard: an untouched entry leaves the mirror unset
        // (the initial update_properties pass ran the map, but the knob was never set).
        EXPECT_EQ(platform->cursor_color, std::nullopt);

        pc::ios_specific::entry::set_cursor_color(control, maui::graphics::colors::lime);
        EXPECT_EQ(platform->cursor_color, maui::graphics::colors::lime);
    }

    TEST(platform_configuration_wired, entry_adjusts_font_size_to_fit_width_reaches_platform_mirror)
    {
        maui::controls::entry control;
        auto handler = std::make_shared<maui::core::entry_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // TextExtensions.UpdateAdjustsFontSizeToFitWidth pushes UNCONDITIONALLY (no IsSet guard): the
        // initial update_properties pass already recorded the knob default (false).
        EXPECT_FALSE(platform->adjusts_font_size_to_fit_width);

        pc::ios_specific::entry::set_adjusts_font_size_to_fit_width(control, true);
        EXPECT_TRUE(platform->adjusts_font_size_to_fit_width);
        pc::ios_specific::entry::set_adjusts_font_size_to_fit_width(control, false);
        EXPECT_FALSE(platform->adjusts_font_size_to_fit_width);
    }

    TEST(platform_configuration_wired, navigation_bar_translucent_reaches_platform_mirror)
    {
        maui::controls::navigation_page page;
        auto handler = std::make_shared<maui::core::navigation_page_handler>();
        page.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_FALSE(platform->bar_translucent);

        pc::ios_specific::navigation_page::set_is_navigation_bar_translucent(page, true);
        EXPECT_TRUE(platform->bar_translucent);
        pc::ios_specific::navigation_page::set_is_navigation_bar_translucent(page, false);
        EXPECT_FALSE(platform->bar_translucent);
    }

    TEST(platform_configuration_wired, page_specific_changes_request_appearance_updates)
    {
        maui::controls::content_page page;
        auto handler = std::make_shared<maui::core::content_page_handler>();
        page.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        const int status_baseline = platform->status_bar_appearance_requests; // attach ran the maps once
        const int home_baseline = platform->home_indicator_requests;

        pc::ios_specific::page::set_prefers_status_bar_hidden(page,
                                                              pc::ios_specific::status_bar_hidden_mode::true_mode);
        EXPECT_EQ(platform->status_bar_appearance_requests, status_baseline + 1);

        pc::ios_specific::page::set_prefers_home_indicator_auto_hidden(page, true);
        EXPECT_EQ(platform->home_indicator_requests, home_baseline + 1);
    }

    TEST(platform_configuration_wired, content_page_implements_ios_page_specifics)
    {
        maui::controls::content_page page;
        const maui::core::i_ios_page_specifics& specifics = page;
        EXPECT_FALSE(specifics.is_home_indicator_auto_hidden());
        EXPECT_EQ(specifics.prefers_status_bar_hidden_mode(), 0);
        EXPECT_EQ(specifics.preferred_status_bar_update_animation_mode(), 0);

        pc::ios_specific::page::set_prefers_home_indicator_auto_hidden(page, true);
        pc::ios_specific::page::set_prefers_status_bar_hidden(page,
                                                              pc::ios_specific::status_bar_hidden_mode::false_mode);
        pc::ios_specific::page::set_preferred_status_bar_update_animation(
            page, pc::ios_specific::ui_status_bar_animation::fade);
        EXPECT_TRUE(specifics.is_home_indicator_auto_hidden());
        EXPECT_EQ(specifics.prefers_status_bar_hidden_mode(), 2);
        EXPECT_EQ(specifics.preferred_status_bar_update_animation_mode(), 2);
    }

    TEST(platform_configuration_wired, page_specifics_redirect_to_parent_page_with_home_indicator_set)
    {
        // Page.cs's parent-redirect quirk: when the page sits in a parent page that has the HOME-INDICATOR
        // knob set, ALL THREE getters consult the parent (the status-bar ones probe that same key).
        maui::controls::content_page child;
        maui::controls::navigation_page parent{child}; // push parents the child page

        const maui::core::i_ios_page_specifics& specifics = child;
        pc::ios_specific::page::set_prefers_status_bar_hidden(child,
                                                              pc::ios_specific::status_bar_hidden_mode::true_mode);
        EXPECT_EQ(specifics.prefers_status_bar_hidden_mode(), 1); // no parent knob set → own value

        pc::ios_specific::page::set_prefers_home_indicator_auto_hidden(parent, true);
        pc::ios_specific::page::set_prefers_status_bar_hidden(parent,
                                                              pc::ios_specific::status_bar_hidden_mode::false_mode);
        EXPECT_TRUE(specifics.is_home_indicator_auto_hidden());
        EXPECT_EQ(specifics.prefers_status_bar_hidden_mode(), 2); // the parent's value wins
    }

    TEST(platform_configuration_wired, use_safe_area_insets_the_content_arrange)
    {
        // The MauiView.AdjustForSafeArea analog: with UseSafeArea set, the realized safe-area insets join
        // the padding in MeasureContent/ArrangeContent's inset; without it the page is edge-to-edge.
        maui::controls::content_page page;
        maui::controls::button body;
        page.set_content(body);
        page.set_padding(maui::core::thickness{10});

        const maui::core::i_safe_area_view& safe_area_face = page;
        maui::core::i_safe_area_view2& insets_face = page;
        EXPECT_TRUE(safe_area_face.ignore_safe_area()); // default: UseSafeArea false → ignore

        insets_face.set_safe_area_insets(maui::core::thickness{0, 59, 0, 34}); // the host's write-back
        page.arrange(maui::graphics::rect{0, 0, 200, 400});
        EXPECT_EQ(body.frame(), (maui::graphics::rect{10, 10, 180, 380})); // padding only

        pc::ios_specific::page::set_use_safe_area(page, true);
        EXPECT_FALSE(safe_area_face.ignore_safe_area());
        page.arrange(maui::graphics::rect{0, 0, 200, 400});
        EXPECT_EQ(body.frame(), (maui::graphics::rect{10, 69, 180, 287})); // padding + safe area

        // And the write-back landed in the read-only knob (Page.cs ISafeAreaView2.SafeAreaInsets set).
        EXPECT_EQ(pc::ios_specific::page::get_safe_area_insets(page), (maui::core::thickness{0, 59, 0, 34}));
    }

    // ================= GTKSpecific (stored-inert; no GTK backend) =====================================

    TEST(platform_configuration_gtk, navigation_and_tabbed_page)
    {
        maui::controls::navigation_page nav;
        EXPECT_EQ(pc::gtk_specific::navigation_page::get_back_button_icon(nav), "");
        pc::gtk_specific::navigation_page::set_back_button_icon(nav.on<pc::gtk>(), "back.png");
        EXPECT_EQ(pc::gtk_specific::navigation_page::get_back_button_icon(nav.on<pc::gtk>()), "back.png");

        maui::controls::tabbed_page tabs;
        using pc::gtk_specific::tab_position;
        EXPECT_EQ(pc::gtk_specific::tabbed_page::get_tab_position(tabs), tab_position::default_position);
        pc::gtk_specific::tabbed_page::set_tab_position(tabs.on<pc::gtk>(), tab_position::bottom);
        EXPECT_EQ(pc::gtk_specific::tabbed_page::get_tab_position(tabs.on<pc::gtk>()), tab_position::bottom);
    }
} // namespace
