// Proves the runtime XAML loader hydrates a full example PAGE — the "with-XAML" example path. The same
// content_page > vertical_stack_layout > {label, button} tree the maui::ui builder produces by hand
// (examples/counter) is here driven entirely from markup, so an example can ship a .xaml alongside its
// hand-written builder twin. (The compile-time codegen path will emit the builder form from this same XAML.)

#include "maui/xaml/xaml_loader.hpp"

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

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
} // namespace
