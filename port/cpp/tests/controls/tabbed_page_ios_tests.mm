// iOS (UIKit) backend tests for the tabbed_page seam, run ON the simulator. The host is a real
// UITabBarController: one child UIViewController per page (vc.view IS the page's native UIView — the
// child-VC composition the W1-10 task asserts), tabBarItem titles from each page's Title,
// selectedIndex tracking CurrentPage both ways (programmatic + the UITabBarControllerDelegate's
// didSelectViewController, invoked directly here to simulate the user tap — UIKit only calls it for
// user selections), and the bar styling painted on the real UITabBar. Compiled as Objective-C++ with
// ARC for the `ios` backend.
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/brushes/linear_gradient_brush.hpp"
#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"

#include "ios_visual_ops.hpp" // k_bar_background_layer_name — the painted bar-background layer's tag

#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::gradient_stop;
    using maui::controls::linear_gradient_brush;
    using maui::controls::solid_color_brush;
    using maui::controls::tabbed_page;
    using maui::core::content_page_handler;
    using maui::core::tabbed_page_handler;

    // The bar-background layer the handler paints (tagged k_bar_background_layer_name), inserted at the
    // bottom (index 0) of the tab bar's layer — nil when none was installed.
    CALayer* bar_background_layer(UITabBar* bar)
    {
        for (CALayer* const sub in bar.layer.sublayers)
        {
            if ([sub.name isEqualToString:maui::platform::ios::k_bar_background_layer_name])
            {
                return sub;
            }
        }
        return nil;
    }

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UITabBarController* native_controller(const std::shared_ptr<tabbed_page_handler>& handler)
    {
        return (__bridge UITabBarController*)handler->typed_platform_view()->controller;
    }

    // A content_page with its handler attached, so it owns a real native UIView host the child view
    // controller can adopt. Returns the page's native UIView for hierarchy assertions.
    UIView* attach_page(content_page& page)
    {
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        return (__bridge UIView*)page_handler->native_view();
    }

    TEST(ios_tabbed_page_seam, controller_hosts_one_child_vc_per_page_with_titles)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        content_page second;
        second.set_title("Second");
        UIView* const first_native = attach_page(first);
        UIView* const second_native = attach_page(second);
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);

        UITabBarController* const controller = native_controller(handler);
        ASSERT_NE(controller, nil);
        ASSERT_EQ(controller.viewControllers.count, 2U);
        // Child-VC composition: each page's native UIView is its wrapper view controller's view.
        EXPECT_EQ(controller.viewControllers[0].view, first_native);
        EXPECT_EQ(controller.viewControllers[1].view, second_native);
        EXPECT_EQ(to_std_string(controller.viewControllers[0].tabBarItem.title), "First");
        EXPECT_EQ(to_std_string(controller.viewControllers[1].tabBarItem.title), "Second");
        // The handler's native root is the controller's view.
        EXPECT_EQ((__bridge UIView*)handler->typed_platform_view()->native, controller.view);
    }

    TEST(ios_tabbed_page_seam, selected_index_tracks_current_page_both_ways)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        content_page second;
        second.set_title("Second");
        attach_page(first);
        attach_page(second);
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBarController* const controller = native_controller(handler);

        // Virtual -> native: the first child became current on add.
        EXPECT_EQ(controller.selectedIndex, 0U);
        tabs.set_current_page(&second);
        EXPECT_EQ(controller.selectedIndex, 1U);
        EXPECT_EQ(handler->typed_platform_view()->selected_index, 1);

        // Native -> virtual: a user tab tap lands in the delegate's didSelectViewController (invoked
        // directly — UIKit only fires it for user selections).
        controller.selectedIndex = 0;
        [controller.delegate tabBarController:controller didSelectViewController:controller.viewControllers[0]];
        EXPECT_EQ(tabs.current_page(), &first);
        EXPECT_EQ(handler->typed_platform_view()->selected_index, 0);
    }

    TEST(ios_tabbed_page_seam, switching_current_page_keeps_the_controller_hierarchy)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        content_page second;
        second.set_title("Second");
        UIView* const first_native = attach_page(first);
        UIView* const second_native = attach_page(second);
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBarController* const controller = native_controller(handler);

        tabs.set_current_page(&second);

        // Both children remain in the controller hierarchy after the switch; the selected child VC is
        // the second page's wrapper.
        ASSERT_EQ(controller.viewControllers.count, 2U);
        EXPECT_EQ(controller.viewControllers[0].view, first_native);
        EXPECT_EQ(controller.viewControllers[1].view, second_native);
        EXPECT_EQ(controller.selectedViewController, controller.viewControllers[1]);
        EXPECT_EQ(handler->typed_platform_view()->hosted_current, &second);
    }

    TEST(ios_tabbed_page_seam, pages_and_title_changes_rebuild_the_children)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBarController* const controller = native_controller(handler);
        ASSERT_EQ(controller.viewControllers.count, 1U);

        content_page second;
        second.set_title("Second");
        attach_page(second);
        tabs.add(second); // PagesChanged -> the children rebuild
        ASSERT_EQ(controller.viewControllers.count, 2U);
        EXPECT_EQ(to_std_string(controller.viewControllers[1].tabBarItem.title), "Second");

        second.set_title("Renamed"); // the Title wiring -> the children rebuild again
        EXPECT_EQ(to_std_string(controller.viewControllers[1].tabBarItem.title), "Renamed");
    }

    TEST(ios_tabbed_page_seam, bar_styling_paints_the_real_tab_bar)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBar* const bar = native_controller(handler).tabBar;

        tabs.set_bar_background_color(maui::graphics::colors::red);
        tabs.set_selected_tab_color(maui::graphics::colors::blue);
        tabs.set_unselected_tab_color(maui::graphics::colors::gray);

        // BarBackgroundColor → the appearance background; Selected/Unselected → the tint pair.
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        [bar.standardAppearance.backgroundColor getRed:&red green:&green blue:&blue alpha:&alpha];
        EXPECT_DOUBLE_EQ(red, 1.0);
        EXPECT_DOUBLE_EQ(green, 0.0);
        EXPECT_DOUBLE_EQ(blue, 0.0);
        [bar.tintColor getRed:&red green:&green blue:&blue alpha:&alpha];
        EXPECT_DOUBLE_EQ(blue, 1.0);
        ASSERT_NE(bar.unselectedItemTintColor, nil);

        // The mirrors stay observable too (the shared seam contract).
        ASSERT_TRUE(handler->typed_platform_view()->bar_background_color.has_value());
        EXPECT_EQ(*handler->typed_platform_view()->bar_background_color, maui::graphics::colors::red);
    }

    // TabbedPage.BarBackground (Brush): a LinearGradientBrush paints an axial CAGradientLayer inserted at
    // the bottom of the tab bar's layer (BrushExtensions.UpdateBackground); clearing it removes the layer.
    TEST(ios_tabbed_page_seam, bar_background_brush_paints_a_gradient_layer)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBar* const bar = native_controller(handler).tabBar;

        EXPECT_EQ(bar_background_layer(bar), nil); // nothing painted until a brush is set

        // U18: a native tab-bar background color must be CLEARED when a brush layer is inserted (C#
        // BrushExtensions.UpdateBackground sets control.BackgroundColor = UIColor.Clear), so it cannot show
        // through the brush. Seed an opaque native color and confirm the brush apply clears it.
        bar.backgroundColor = [UIColor greenColor];

        auto brush = std::make_shared<linear_gradient_brush>(
            std::vector<std::shared_ptr<gradient_stop>>{
                std::make_shared<gradient_stop>(maui::graphics::colors::red, 0.0F),
                std::make_shared<gradient_stop>(maui::graphics::colors::blue, 1.0F)},
            maui::graphics::point{0, 0}, maui::graphics::point{1, 1});
        tabs.set_bar_background(brush);

        EXPECT_EQ(bar.backgroundColor, nil) << "applying a bar background brush must clear the UITabBar's native color";

        CALayer* const layer = bar_background_layer(bar);
        ASSERT_NE(layer, nil);
        // The gradient layer is the bottom-most sublayer (inserted at index 0).
        EXPECT_EQ(bar.layer.sublayers.firstObject, layer);
        auto* const gradient = static_cast<CAGradientLayer*>(layer);
        ASSERT_TRUE([gradient isKindOfClass:[CAGradientLayer class]]);
        EXPECT_TRUE([gradient.type isEqualToString:kCAGradientLayerAxial]);
        ASSERT_EQ(gradient.colors.count, 2U);
        ASSERT_EQ(gradient.locations.count, 2U);
        EXPECT_DOUBLE_EQ(gradient.locations[0].doubleValue, 0.0);
        EXPECT_DOUBLE_EQ(gradient.locations[1].doubleValue, 1.0);

        // The mirror tracks the brush too (value_or avoids an unchecked optional access the gtest ASSERT
        // is invisible to).
        ASSERT_TRUE(handler->typed_platform_view()->bar_background_brush.has_value());
        EXPECT_EQ(handler->typed_platform_view()->bar_background_brush.value_or(nullptr).get(), brush.get());

        // Clearing the brush removes the painted layer (null/empty restores the default).
        tabs.set_bar_background(nullptr);
        EXPECT_EQ(bar_background_layer(bar), nil);
        EXPECT_FALSE(handler->typed_platform_view()->bar_background_brush.has_value());
    }

    // A SolidColorBrush paints a plain CALayer (not a gradient) carrying the color; switching to a
    // gradient and back never leaves a stale layer behind (the remove-before-insert lifecycle).
    TEST(ios_tabbed_page_seam, bar_background_brush_solid_then_gradient_replaces_cleanly)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBar* const bar = native_controller(handler).tabBar;

        tabs.set_bar_background(std::make_shared<solid_color_brush>(maui::graphics::colors::green));
        CALayer* const solid = bar_background_layer(bar);
        ASSERT_NE(solid, nil);
        EXPECT_FALSE([solid isKindOfClass:[CAGradientLayer class]]);
        ASSERT_NE(solid.backgroundColor, nullptr);

        // Replace with a gradient: exactly one tagged layer remains, and it is the gradient.
        tabs.set_bar_background(std::make_shared<linear_gradient_brush>(std::vector<std::shared_ptr<gradient_stop>>{
            std::make_shared<gradient_stop>(maui::graphics::colors::red, 0.0F),
            std::make_shared<gradient_stop>(maui::graphics::colors::blue, 1.0F)}));
        int tagged = 0;
        for (CALayer* const sub in bar.layer.sublayers)
        {
            if ([sub.name isEqualToString:maui::platform::ios::k_bar_background_layer_name])
            {
                ++tagged;
            }
        }
        EXPECT_EQ(tagged, 1);
        ASSERT_TRUE([bar_background_layer(bar) isKindOfClass:[CAGradientLayer class]]);
    }

    // An EMPTY brush (Brush.IsNullOrEmpty: a SolidColorBrush with a null color) clears the layer rather
    // than painting — C# `if (Brush.IsNullOrEmpty(brush)) return;` after the removal (a value-type paint
    // would otherwise paint opaque black for the null color).
    TEST(ios_tabbed_page_seam, bar_background_empty_brush_clears_the_layer)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBar* const bar = native_controller(handler).tabBar;

        // First paint a real fill so there is a layer to clear.
        tabs.set_bar_background(std::make_shared<solid_color_brush>(maui::graphics::colors::green));
        ASSERT_NE(bar_background_layer(bar), nil);

        // An empty solid brush (null color) is IsNullOrEmpty → the layer is removed, nothing painted.
        tabs.set_bar_background(std::make_shared<solid_color_brush>());
        EXPECT_EQ(bar_background_layer(bar), nil);
    }

    // Gradient stop changes repaint through InvalidateGradientBrushRequested (the renderer subscribes to a
    // GradientBrush bar background and re-applies on every stop mutation).
    TEST(ios_tabbed_page_seam, bar_background_gradient_repaints_on_stop_change)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBar* const bar = native_controller(handler).tabBar;

        auto brush = std::make_shared<linear_gradient_brush>(std::vector<std::shared_ptr<gradient_stop>>{
            std::make_shared<gradient_stop>(maui::graphics::colors::red, 0.0F)});
        tabs.set_bar_background(brush);

        CALayer* const before = bar_background_layer(bar);
        ASSERT_NE(before, nil);
        ASSERT_EQ(static_cast<CAGradientLayer*>(before).colors.count, 1U);

        // Adding a stop fires InvalidateGradientBrushRequested → the bar repaints with both colors.
        brush->gradient_stops().add(std::make_shared<gradient_stop>(maui::graphics::colors::blue, 1.0F));
        CALayer* const after = bar_background_layer(bar);
        ASSERT_NE(after, nil);
        EXPECT_EQ(static_cast<CAGradientLayer*>(after).colors.count, 2U);
    }
} // namespace
