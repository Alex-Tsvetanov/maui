// Proves the runtime XAML loader hydrates a full example PAGE — the "with-XAML" example path. The same
// content_page > vertical_stack_layout > {label, button} tree the maui::ui builder produces by hand
// (examples/counter) is here driven entirely from markup, so an example can ship a .xaml alongside its
// hand-written builder twin. (The compile-time codegen path will emit the builder form from this same XAML.)

#include "maui/xaml/xaml_loader.hpp"

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/i_container.hpp"
#include "maui/graphics/rect.hpp"

#include <limits>
#include <string>

#include <gtest/gtest.h>

namespace
{
    namespace controls = maui::controls;
    using maui::xaml::xaml_load_result;
    using maui::xaml::xaml_loader;

    TEST(xaml_example_page, loads_counter_shaped_page_from_markup)
    {
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
             Title="Counter">
  <VerticalStackLayout Spacing="12">
    <Label x:Name="count" Text="Count: 0"/>
    <Button x:Name="increment" Text="Increment"/>
  </VerticalStackLayout>
</ContentPage>)xml");

        EXPECT_EQ(page.title(), "Counter"); // ContentPage.Title attribute applied

        const std::shared_ptr<controls::label> count = result.find_by_name<controls::label>("count");
        ASSERT_NE(count, nullptr);
        EXPECT_EQ(count->text(), "Count: 0");

        const std::shared_ptr<controls::button> increment = result.find_by_name<controls::button>("increment");
        ASSERT_NE(increment, nullptr);
        EXPECT_EQ(increment->text(), "Increment");
    }

    // ---- regression: a shape's explicit WidthRequest/HeightRequest with NO HorizontalOptions must
    // collapse Fill -> Center (MAUI LayoutExtensions.AlignHorizontal), matching MAUI's rendered
    // reference for a bare Rectangle in a StackLayout. This drives the REAL runtime XAML loader
    // (register_xaml_shapes registrations + the generic StackLayout) with hand-inlined markup — not a
    // hand-built tree — to prove the Fill+explicit-size->Center rule holds through the production XAML
    // path for shape controls specifically (shapes override measure() but must still honor
    // view<>::compute_frame's alignment resolution). ----
    TEST(xaml_example_page, shape_with_no_horizontal_options_centers_in_stack_layout)
    {
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             x:Class="MauiReference.Pages.ShapeAppThemePage" Title="Shapes AppTheme Gallery">
    <StackLayout Padding="12"
                 BackgroundColor="White">
        <Label Text="Shape using AppTheme" />
        <Rectangle WidthRequest="200"
                   HeightRequest="80"
                   Fill="Green"
                   Stroke="Green" />
    </StackLayout>
</ContentPage>)xml");
        (void)result;

        auto* stack = dynamic_cast<controls::stack_layout*>(page.content());
        ASSERT_NE(stack, nullptr);
        ASSERT_EQ(stack->count(), 2);

        auto* rect = dynamic_cast<controls::shapes::rectangle*>(&stack->at(1));
        ASSERT_NE(rect, nullptr);

        constexpr double inf = std::numeric_limits<double>::infinity();
        page.measure(400, inf);
        page.arrange(maui::graphics::rect(0, 0, 400, 300));

        // The stack's content band is 400 - 2*12 padding = 376 wide; a 200-wide Fill+explicit-width
        // Rectangle with no HorizontalOptions must be CENTERED within it.
        EXPECT_EQ(rect->frame().width, 200);
        EXPECT_EQ(rect->frame().x, 12 + (376 - 200) / 2.0); // padding.left + centered offset
    }

    // ---- regression (the actual shape_app_theme.xaml fix): the real C# ShapeAppThemeGallery.xaml sets
    // HorizontalOptions="Start" on its Rectangle (src/Controls/samples/.../ShapeAppThemeGallery.xaml),
    // and port/maui-reference/pages/shape_app_theme.xaml now mirrors that. Before this fix,
    // register_xaml_shapes.cpp carried its OWN local register_view_properties<T> duplicate that never
    // registered "HorizontalOptions" (or Margin/VerticalOptions/FlowDirection/Clip/Background*/Style) for
    // any shape type, so a shape's HorizontalOptions attribute was silently DROPPED by the XAML loader —
    // it would render Center (the Fill default) no matter what the markup said. This test drives the
    // real xaml_loader with HorizontalOptions="Start" on a Rectangle and asserts it lands flush-LEFT
    // (frame.x == the stack's left padding, not centered), proving the shared register_xaml_helpers.hpp
    // register_view_properties<T> is now actually reached for shapes. ----
    TEST(xaml_example_page, shape_with_horizontal_options_start_is_left_aligned_in_stack_layout)
    {
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             x:Class="MauiReference.Pages.ShapeAppThemePage" Title="Shapes AppTheme Gallery">
    <StackLayout Padding="12"
                 BackgroundColor="White">
        <Label Text="Shape using AppTheme" />
        <Rectangle HorizontalOptions="Start"
                   WidthRequest="200"
                   HeightRequest="80"
                   Fill="Green"
                   Stroke="Green" />
    </StackLayout>
</ContentPage>)xml");
        (void)result;

        auto* stack = dynamic_cast<controls::stack_layout*>(page.content());
        ASSERT_NE(stack, nullptr);
        ASSERT_EQ(stack->count(), 2);

        auto* rect = dynamic_cast<controls::shapes::rectangle*>(&stack->at(1));
        ASSERT_NE(rect, nullptr);

        constexpr double inf = std::numeric_limits<double>::infinity();
        page.measure(400, inf);
        page.arrange(maui::graphics::rect(0, 0, 400, 300));

        // HorizontalOptions="Start" must be honored (NOT silently dropped): the Rectangle sits flush at
        // the stack's left padding edge, not centered in the 376-wide content band.
        EXPECT_EQ(rect->frame().width, 200);
        EXPECT_EQ(rect->frame().x, 12); // padding.left only -- flush left, not (376-200)/2 centered
    }
} // namespace
