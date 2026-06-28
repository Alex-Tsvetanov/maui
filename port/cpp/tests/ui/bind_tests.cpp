// Tests for typed ui::bind (PUBLIC_API_DESIGN.md §3-E): one-way and two-way binding between a control and a
// view-model observable<T>, with writes routed through the control's real set_* and the typed source making
// a wrong name a compile error.

#include "maui/ui/bind.hpp"

#include "maui/controls/button.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/observable.hpp"

#include <string>

#include <gtest/gtest.h>

namespace
{
    class greeting_vm : public maui::core::bindable_object
    {
    public:
        maui::core::observable<std::string> message{*this, "message", "World"};
    };

    class flag_vm : public maui::core::bindable_object
    {
    public:
        maui::core::observable<bool> enabled{*this, "enabled", true};
    };

    TEST(ui_bind, one_way_vm_to_control)
    {
        greeting_vm vm;
        maui::controls::label lbl;
        vm.message.set("Hello");

        auto handle = maui::ui::bind(lbl, &maui::controls::label::set_text).to(vm.message);
        EXPECT_EQ(std::string(lbl.text()), "Hello"); // initial push at bind time

        vm.message.set("World");
        EXPECT_EQ(std::string(lbl.text()), "World"); // source change propagates to the control
    }

    TEST(ui_bind, two_way_control_and_vm)
    {
        greeting_vm vm;
        maui::controls::entry input;
        vm.message.set("start");

        auto handle =
            maui::ui::bind(input, &maui::controls::entry::set_text, &maui::controls::entry::text, &input.text_changed)
                .to_two_way(vm.message);
        EXPECT_EQ(std::string(input.text()), "start"); // initial push vm -> control

        // Source -> control: a VM change flows to the entry.
        vm.message.set("external");
        EXPECT_EQ(std::string(input.text()), "external");

        // Control -> source: simulate the user typing (the handler updates the text, then raises the event).
        input.set_text("typed");
        input.send_text_changed("external", "typed");
        EXPECT_EQ(vm.message.get(), "typed");
    }

    TEST(ui_bind, one_way_to_an_inherited_property)
    {
        // is_enabled is declared on the view<> base, so &button::set_is_enabled is a BASE-class member pointer;
        // ui::bind binds it against the derived button anyway (the inherited-property case).
        flag_vm vm;
        maui::controls::button btn;
        vm.enabled.set(false);

        auto handle = maui::ui::bind(btn, &maui::controls::button::set_is_enabled).to(vm.enabled);
        EXPECT_FALSE(btn.is_enabled()); // initial push (false)

        vm.enabled.set(true);
        EXPECT_TRUE(btn.is_enabled()); // source change propagates through the inherited setter
    }
} // namespace
