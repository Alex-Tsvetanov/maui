#pragma once
// maui::samples::triggers_page — ports TriggersPage.xaml.
//
// The C# gallery page demonstrates an EventTrigger: an Entry whose TextChanged event runs a
// NumericValidationTriggerAction that turns the entry's TextColor red when the text is not a valid
// System.Double, and restores it otherwise.
//
// This port reproduces that behavior with the port's TRIGGER MACHINERY, code-first, on the headless-safe
// `maui::` surface. The faithful seam: a property_trigger<bool> watches a page-owned property<bool>
// (invalid_) and, while it holds, applies a setter recoloring the entry's text red — un-applying it
// (restoring the color beneath) when validity returns. The validity flag is driven from the entry's public
// text_changed event (the EventTrigger's TextChanged source), parsing the new text as a double:
//
//   entry.text_changed  ->  invalid_ := !parses_as_double(new_text)  ->  property_trigger flips the
//   red-text setter on/off at setter_specificity::trigger (one rung above a manual set, the C# precedence).
//
// This mirrors the NumericValidationTriggerAction logic (valid double => default color; invalid => red)
// while exercising the typed property_trigger seam the prompt asks for (a property value changing a
// control's appearance). A second showcase drives the same trigger machinery off a button: a "toggle
// highlight" button flips a property<bool> that a property_trigger watches to recolor a status label —
// the minimal property-driven trigger the port's trigger.hpp tests use.
//
// To OWN the watched property<bool> slots the property_triggers observe, the page itself derives from
// maui::core::bindable_object (the same role the trigger tests' mock_object plays) — the property<bool>
// constructor needs a bindable owner, and `*this` is that owner.
//
// note: the port's entry does NOT register a "text_changed" NAMED event channel (only button registers
// clicked/pressed/released), so a literal event_trigger over "text_changed" would attach nothing here.
// Subscribing the entry's public text_changed event directly is the equivalent reflection-free wiring and
// keeps the validation behavior identical; the trigger itself (the appearance change) is a real
// property_trigger, faithful to the XAML's intent.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window.

#include <cstddef>
#include <cstdio>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/trigger.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    // Derives from bindable_object so the page can host the watched property<bool> slots the
    // property_triggers observe (the trigger tests' mock_object role).
    class triggers_page : public maui::core::bindable_object
    {
    public:
        triggers_page()
        {
            page_.set_title("Triggers");
            stack_.set_spacing(12);

            heading_.set_text("Triggers");
            instructions_.set_text("Text must be a valid double or it will turn red.");

            entry_.set_placeholder("Enter a System.Double");
            // The EventTrigger's source: drive the validity flag from the entry's TextChanged. An empty
            // field is treated as valid (the rest state), matching the page's at-rest appearance.
            entry_.text_changed.connect([this](const std::string& /*old_text*/, const std::string& new_text) {
                invalid_.set(!is_valid_double(new_text));
            });

            // The validation trigger (the NumericValidationTriggerAction analog): while `invalid_` holds,
            // recolor the entry's text red; restore it otherwise. The setter targets the entry directly
            // (Setter.TargetName retarget via setter::of_for) since the watched property lives on the page.
            validation_trigger_.add(maui::controls::setter::of_for(entry_, maui::controls::entry::text_color_property(),
                                                                   maui::graphics::color::from_rgb(220, 30, 30)));
            validation_handle_ = validation_trigger_.attach(*this);

            // A second showcase: a property-driven trigger recoloring a status label, toggled by a button —
            // the minimal "property value changes a control's appearance" the trigger seam demonstrates.
            status_.set_text("Highlight off");
            highlight_trigger_.add(maui::controls::setter::of_for(status_, maui::controls::label::text_color_property(),
                                                                  maui::graphics::color::from_rgb(20, 130, 40)));
            highlight_handle_ = highlight_trigger_.attach(*this);

            toggle_button_.set_text("Toggle highlight");
            toggle_button_.clicked.connect([this] { highlighted_.set(!highlighted_.get()); });

            stack_.add(heading_);
            stack_.add(instructions_);
            stack_.add(entry_);
            stack_.add(status_);
            stack_.add(toggle_button_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& v, const char* n) {
                try
                {
                    app.attach_handler(v);
                }
                catch (const std::exception& e)
                {
                    std::fprintf(stderr, "[gallery] skip %s: %s\n", n, e.what());
                }
            };
            one(heading_, "heading_");
            one(instructions_, "instructions_");
            one(entry_, "entry_");
            one(status_, "status_");
            one(toggle_button_, "toggle_button_");
            one(stack_, "stack_");
            one(page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // Owned controls / state, exposed for tests / the hosting main.
        [[nodiscard]] maui::controls::entry& validated_entry()
        {
            return entry_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }
        [[nodiscard]] maui::core::property<bool>& invalid()
        {
            return invalid_;
        }
        [[nodiscard]] maui::core::property<bool>& highlighted()
        {
            return highlighted_;
        }

    private:
        static const maui::core::bindable_property<bool>& invalid_property()
        {
            static const maui::core::bindable_property<bool> descriptor{"invalid", false};
            return descriptor;
        }
        static const maui::core::bindable_property<bool>& highlighted_property()
        {
            static const maui::core::bindable_property<bool> descriptor{"highlighted", false};
            return descriptor;
        }

        // NumericValidationTriggerAction's core: does the text parse as a double? An empty string is valid
        // (rest state). The whole token (ignoring trailing whitespace) must be consumed, mirroring the
        // all-or-nothing intent of C#'s double.TryParse.
        [[nodiscard]] static bool is_valid_double(const std::string& text)
        {
            if (text.empty())
            {
                return true;
            }
            try
            {
                std::size_t consumed = 0;
                (void)std::stod(text, &consumed);
                while (consumed < text.size() && (text[consumed] == ' ' || text[consumed] == '\t'))
                {
                    ++consumed;
                }
                return consumed == text.size();
            }
            catch (...)
            {
                return false;
            }
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label heading_;
        maui::controls::label instructions_;
        maui::controls::entry entry_;
        maui::controls::label status_;
        maui::controls::button toggle_button_;

        // The watched flags — declared BEFORE the triggers that reference them (member-init order). Owned by
        // `*this` (the page's bindable_object base).
        maui::core::property<bool> invalid_{*this, invalid_property()};
        maui::core::property<bool> highlighted_{*this, highlighted_property()};

        // The triggers + their RAII handles. invalid_ drives the entry's red text; highlighted_ drives the
        // status label's green text.
        maui::controls::property_trigger<bool> validation_trigger_{invalid_, true};
        maui::controls::property_trigger<bool> highlight_trigger_{highlighted_, true};
        maui::controls::trigger_handle validation_handle_;
        maui::controls::trigger_handle highlight_handle_;
    };
} // namespace maui::samples
