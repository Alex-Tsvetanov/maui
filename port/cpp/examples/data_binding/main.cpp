// data_binding — a view-model bound to controls, kept in sync automatically (idiomatic maui::ui).
//
// ONE concept: TYPED binding. The view-model holds a maui::core::observable<std::string> — ONE member, no
// descriptor boilerplate. `ui::bind(control, &Ctrl::set_x).to(vm.obs)` wires a one-way binding that routes
// writes through the control's real set_*; the two-way form (.to_two_way) also takes the getter + change
// event, so the entry drives the view-model and the view-model drives the greeting label. A wrong property
// is a COMPILE error here, not a silently dead string binding. The tree is built with the owning builder.
//
// 100% PORTABLE C++: no platform headers.

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding.hpp"

#include <string>

namespace ui = maui::ui;

// The view-model: a bindable_object with one observable property — no static descriptor, no member wiring.
class greeting_view_model : public maui::core::bindable_object
{
public:
    maui::core::observable<std::string> message{*this, "message", "World"};
};

class data_binding_app : public ui::app
{
public:
    data_binding_app()
    {
        auto prompt = ui::label("Type a name; the greeting updates live:");
        auto input = ui::entry();
        input->set_placeholder("name"); // long-tail property via the operator-> escape
        auto greeting = ui::label();

        // Typed bindings, wired while we still hold the named handles (the controls are heap-stable, so the
        // references survive the move into the builder). greeting tracks the view-model one-way; the entry
        // is two-way (it both reads from and writes to vm.message).
        greeting_binding_ = ui::bind(greeting.impl(), &maui::controls::label::set_text).to(view_model_.message);
        input_binding_ = ui::bind(input.impl(), &maui::controls::entry::set_text, &maui::controls::entry::text,
                                  &input.impl().text_changed)
                             .to_two_way(view_model_.message);

        set_content(ui::page(ui::vstack(std::move(prompt), std::move(input), std::move(greeting))
                                 .spacing(12)
                                 .padding(ui::thickness{16.0})));
        set_title("Data Binding");
    }

private:
    greeting_view_model view_model_;
    // Binding handles are DERIVED members, so they destruct before ui::app's (base) content root + view-model:
    // they disconnect while the controls and the view-model are still alive.
    maui::core::binding_handle greeting_binding_;
    maui::core::binding_handle input_binding_;
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<data_binding_app>();
    return builder;
}
