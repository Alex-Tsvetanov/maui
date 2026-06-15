#pragma once
// maui::controls::search_bar  <=  Microsoft.Maui.Controls.SearchBar
//
// A specialized input control for entering search text. Ported from SearchBar.cs + InputView.cs. Same
// API shape as entry/editor: bare-noun interface getters + method accessors, each backed by a private
// property<T> whose change flows through view::on_property_changed to the handler (virtual→native).
//
// text_changed semantics (SearchBarUnitTests.TestContentsChanged / SearchBarTextChangedEventArgs): in
// C# BOTH a programmatic Text set and a native edit funnel through the Text property's propertyChanged,
// so set_text raises text_changed with (old, new); a native edit arrives through send_text_changed (the
// native bar is the source of truth during a live edit, so it raises directly without re-pushing
// through the property store).
//
// search_button_pressed: C#'s SearchCommand/SearchCommandParameter (ICommand) collapse to the port's
// event channel (documented deviation — there is no ICommand subsystem): send_search_button_pressed()
// raises the `search_button_pressed` event unconditionally, matching OnSearchButtonPressed with no
// command attached. The optional `search_command` move_only_function mirrors the button's `command`
// convention and runs before the event (the C# command-then-event order).
//
// Defaults (SearchBar.cs): ReturnType.Search; CancelButtonColor/SearchIconColor default-color ("null" —
// platform default); HorizontalTextAlignment Start, VerticalTextAlignment Center (TextAlignmentElement).
//
// Deferred (OUT OF SCOPE this cut, documented not stubbed): TextTransform, the ICommand CanExecute →
// IsEnabledCore coupling.

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_search_bar.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class search_bar : public view<maui::core::i_search_bar>
    {
    public:
        // Declare the style TargetType so an implicit / class style targeting `search_bar` matches.
        search_bar()
        {
            this->set_style_target_type<search_bar>();
        }

        // Shared bindable-property descriptors (one instance per type, like SearchBar.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<std::string>& placeholder_property();
        static const maui::core::bindable_property<maui::graphics::color>& placeholder_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& cancel_button_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& search_icon_color_property();
        static const maui::core::bindable_property<bool>& is_read_only_property();
        static const maui::core::bindable_property<int>& max_length_property();
        static const maui::core::bindable_property<bool>& is_text_prediction_enabled_property();
        static const maui::core::bindable_property<bool>& is_spell_check_enabled_property();
        static const maui::core::bindable_property<int>& cursor_position_property();
        static const maui::core::bindable_property<int>& selection_length_property();
        static const maui::core::bindable_property<maui::core::return_type>& return_type_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& horizontal_text_alignment_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& vertical_text_alignment_property();
        static const maui::core::bindable_property<maui::core::keyboard>& keyboard_property();

        // ---- i_text / i_text_style ----
        [[nodiscard]] std::string_view text() const override
        {
            return text_.get();
        }
        [[nodiscard]] maui::graphics::color text_color() const override
        {
            return text_color_.get();
        }
        [[nodiscard]] maui::core::font font() const override
        {
            return font_.get();
        }
        [[nodiscard]] double character_spacing() const override
        {
            return character_spacing_.get();
        }

        // ---- i_text_input (placeholder + value flags) ----
        [[nodiscard]] std::string_view placeholder() const override
        {
            return placeholder_.get();
        }
        [[nodiscard]] maui::graphics::color placeholder_color() const override
        {
            return placeholder_color_.get();
        }
        [[nodiscard]] bool is_read_only() const override
        {
            return is_read_only_.get();
        }
        [[nodiscard]] int max_length() const override
        {
            return max_length_.get();
        }
        [[nodiscard]] bool is_text_prediction_enabled() const override
        {
            return is_text_prediction_enabled_.get();
        }
        [[nodiscard]] bool is_spell_check_enabled() const override
        {
            return is_spell_check_enabled_.get();
        }
        [[nodiscard]] int cursor_position() const override
        {
            return cursor_position_.get();
        }
        [[nodiscard]] int selection_length() const override
        {
            return selection_length_.get();
        }
        [[nodiscard]] maui::core::keyboard keyboard() const override
        {
            return keyboard_.get();
        }

        // ---- i_text_alignment ----
        [[nodiscard]] maui::core::text_alignment horizontal_text_alignment() const override
        {
            return horizontal_text_alignment_.get();
        }
        [[nodiscard]] maui::core::text_alignment vertical_text_alignment() const override
        {
            return vertical_text_alignment_.get();
        }

        // ---- i_search_bar ----
        [[nodiscard]] maui::graphics::color cancel_button_color() const override
        {
            return cancel_button_color_.get();
        }
        [[nodiscard]] maui::graphics::color search_icon_color() const override
        {
            return search_icon_color_.get();
        }
        [[nodiscard]] maui::core::return_type return_type() const override
        {
            return return_type_.get();
        }

        // ---- i_text_input mutable cursor/selection (inbound; clamp the floor to 0) ----
        void set_cursor_position(int value) override
        {
            cursor_position_.set(value < 0 ? 0 : value);
        }
        void set_selection_length(int value) override
        {
            selection_length_.set(value < 0 ? 0 : value);
        }

        // ---- public setters (drive the handler via on_property_changed → update_value) ----
        // set_text applies max_length truncation, then raises text_changed with (old, new) when the
        // value actually changed (InputView's TextProperty propertyChanged).
        void set_text(std::string value)
        {
            const int limit = max_length_.get();
            if (limit >= 0 && value.size() > static_cast<std::size_t>(limit))
            {
                value.resize(static_cast<std::size_t>(limit));
            }
            const std::string old{text_.get()};
            text_.set(std::move(value));
            if (text_.get() != old)
            {
                text_changed.raise(old, std::string(text_.get()));
            }
        }
        void set_placeholder(std::string value)
        {
            placeholder_.set(std::move(value));
        }
        void set_placeholder_color(maui::graphics::color value)
        {
            placeholder_color_.set(value);
        }
        void set_cancel_button_color(maui::graphics::color value)
        {
            cancel_button_color_.set(value);
        }
        void set_search_icon_color(maui::graphics::color value)
        {
            search_icon_color_.set(value);
        }
        void set_is_read_only(bool value)
        {
            is_read_only_.set(value);
        }
        // Setting a (non-negative) max_length truncates the current text to match (the entry/editor
        // convention for C#'s native UpdateMaxLength re-trim); the truncation routes through set_text so
        // text_changed fires for the trimmed value.
        void set_max_length(int value)
        {
            max_length_.set(value);
            if (value >= 0 && text_.get().size() > static_cast<std::size_t>(value))
            {
                set_text(std::string(text_.get()));
            }
        }
        void set_text_color(maui::graphics::color value)
        {
            text_color_.set(value);
        }
        void set_font(maui::core::font value)
        {
            font_.set(std::move(value));
        }
        void set_character_spacing(double value)
        {
            character_spacing_.set(value);
        }
        void set_horizontal_text_alignment(maui::core::text_alignment value)
        {
            horizontal_text_alignment_.set(value);
        }
        void set_vertical_text_alignment(maui::core::text_alignment value)
        {
            vertical_text_alignment_.set(value);
        }
        void set_is_text_prediction_enabled(bool value)
        {
            is_text_prediction_enabled_.set(value);
        }
        void set_is_spell_check_enabled(bool value)
        {
            is_spell_check_enabled_.set(value);
        }
        void set_return_type(maui::core::return_type value)
        {
            return_type_.set(value);
        }
        void set_keyboard(maui::core::keyboard value)
        {
            keyboard_.set(value);
        }

        // ---- i_search_bar inbound channel (called by the handler on native events) ----
        // SearchBar.OnSearchButtonPressed: run the command (when set), then raise the event. No
        // IsEnabled gate (C# gates only on the command's CanExecute, which the port has no analog for).
        void send_search_button_pressed() override
        {
            if (search_command)
            {
                search_command();
            }
            search_button_pressed.raise();
        }
        void send_text_changed(std::string_view old_value, std::string_view new_value) override
        {
            text_changed.raise(std::string(old_value), std::string(new_value));
        }

        // ---- developer-facing events + command (the outbound channel) ----
        maui::core::event<> search_button_pressed;
        maui::core::event<std::string, std::string> text_changed;
        maui::core::move_only_function<void()> search_command; // the SearchCommand stand-in

    private:
        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<std::string> placeholder_{*this, placeholder_property()};
        maui::core::property<maui::graphics::color> placeholder_color_{*this, placeholder_color_property()};
        maui::core::property<maui::graphics::color> cancel_button_color_{*this, cancel_button_color_property()};
        maui::core::property<maui::graphics::color> search_icon_color_{*this, search_icon_color_property()};
        maui::core::property<bool> is_read_only_{*this, is_read_only_property()};
        maui::core::property<int> max_length_{*this, max_length_property()};
        maui::core::property<bool> is_text_prediction_enabled_{*this, is_text_prediction_enabled_property()};
        maui::core::property<bool> is_spell_check_enabled_{*this, is_spell_check_enabled_property()};
        maui::core::property<int> cursor_position_{*this, cursor_position_property()};
        maui::core::property<int> selection_length_{*this, selection_length_property()};
        maui::core::property<maui::core::return_type> return_type_{*this, return_type_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
        maui::core::property<maui::core::text_alignment> horizontal_text_alignment_{
            *this, horizontal_text_alignment_property()};
        maui::core::property<maui::core::text_alignment> vertical_text_alignment_{*this,
                                                                                  vertical_text_alignment_property()};
        maui::core::property<maui::core::keyboard> keyboard_{*this, keyboard_property()};
    };
} // namespace maui::controls
