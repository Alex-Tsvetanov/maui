// data_binding — a view-model bound to a control, kept in sync automatically.
//
// ONE concept: binding a control property to a view-model property by NAME, the code-first analog of
// MAUI's `<Label Text="{Binding Message}" />`. The pieces:
//   - A view-model is a `bindable_object` holding `property<T>` members. Each property self-registers
//     under its descriptor name (here "message"), so a string-path binding can resolve it — no
//     reflection.
//   - We set the view-model as the label's BindingContext, then `set_binding("text", "message")`. The
//     label's Text now tracks vm.message: change the source and the label re-renders itself.
//   - An entry drives the view-model: on each keystroke the handler raises `text_changed`, and we push
//     the new value into vm.message — which flows through the binding back out to the label.
//
// This is one-directional VM->label binding plus a manual entry->VM push, the smallest end-to-end loop
// that shows the binding actually propagating. 100% PORTABLE C++: no platform headers.

#include "maui/maui_main.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"

#include <memory>
#include <string>

// ---- The view-model: a bindable_object with one named, observable property ----
class greeting_view_model : public maui::core::bindable_object
{
public:
    // The shared descriptor names the property "message" — the exact string a binding path resolves.
    static const maui::core::bindable_property<std::string>& message_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"message"};
        return descriptor;
    }

    // The per-instance value slot. Constructing it self-registers "message" on this object, so
    // set_binding("text", "message") finds it. `.changed` fires on every set, driving bound targets.
    maui::core::property<std::string> message{*this, message_property()};
};

class data_binding_app : public maui::controls::application
{
public:
    data_binding_app()
    {
        view_model_ = std::make_shared<greeting_view_model>();
        view_model_->message.set("World");

        prompt_.set_text("Type a name; the greeting updates live:");
        input_.set_placeholder("name");
        input_.set_text("World");

        // Bind the label's Text to the view-model's "message" property. First give the label the
        // view-model as its binding context, then declare the binding by property name + source path.
        greeting_.set_binding_context(view_model_);
        greeting_.set_binding("text", "message", maui::core::binding_mode::one_way);

        // Drive the view-model from the entry: every edit pushes the new text into vm.message, which the
        // binding propagates out to the label. (The handler passes old + new; we only need the new value.)
        text_token_ = maui::core::connect_scoped(
            input_.text_changed, [this](const std::string& /*old_value*/, const std::string& new_value) {
                view_model_->message.set(new_value);
            });

        root_.set_padding(maui::core::thickness{16.0});
        root_.set_spacing(12.0);
        root_.add(prompt_);
        root_.add(input_);
        root_.add(greeting_);

        page_.set_content(root_);
        window_.set_content(page_);
        window_.set_title("Data Binding");
    }

    maui::core::i_window* create_window() override
    {
        return &window_;
    }

private:
    std::shared_ptr<greeting_view_model> view_model_;

    maui::controls::window window_;
    maui::controls::content_page page_;
    maui::controls::vertical_stack_layout root_;
    maui::controls::label prompt_;
    maui::controls::entry input_;
    maui::controls::label greeting_;
    maui::core::scoped_connection text_token_;
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<data_binding_app>();
    return builder;
}
