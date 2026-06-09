#pragma once
// maui::controls::entry  <=  Microsoft.Maui.Controls.Entry
//
// A single-line text entry — the first editable, inbound-text control. Ported from Entry.cs + InputView.cs.
// Same API shape as button/label: bare-noun interface getters + method accessors, each backed by a private
// property<T> whose change flows through view::on_property_changed to the handler (virtual→native). The
// reverse direction is the send_* methods: on a native edit the handler calls them and the control raises
// its public events.
//
// Inbound semantics (mirroring InputView/Entry):
//   send_text_changed(old, new) raises `text_changed` — matching InputView.OnTextChanged firing TextChanged
//     with (oldValue, newValue). The native field is the source of truth for its own text during a live
//     edit, so this raises the event directly and does NOT re-push through the property store (which would
//     redundantly re-run map_text back onto the field).
//   send_completed() raises `completed` only when enabled — matching Entry.SendCompleted's IsEnabled gate.
//
// max_length: set_text truncates to max_length (C# enforces this natively via UpdateMaxLength; the port
// applies it in the control so the headless backend matches): if max_length >= 0 and the value is longer,
// it is cut to max_length characters before being stored.
//
// cursor_position / selection_length: bindable, default 0, never negative (C#'s validateValue >= 0). The
// public setters clamp the floor to 0; set_cursor_position / set_selection_length are also the inbound
// channel the handler calls when the user moves the native cursor (i_text_input's mutable pair) — which
// re-pushes through the property store so the mapper keeps the native field in sync (idempotent: a set to
// the current value is a no-op in property<T>).
//
// Deferred (OUT OF SCOPE this cut, documented not stubbed): ReturnCommand, Keyboard.

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/property.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class entry : public view<maui::core::i_entry>
    {
    public:
        // Shared bindable-property descriptors (one instance per type, like Entry.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<std::string>& placeholder_property();
        static const maui::core::bindable_property<maui::graphics::color>& placeholder_color_property();
        static const maui::core::bindable_property<bool>& is_password_property();
        static const maui::core::bindable_property<bool>& is_read_only_property();
        static const maui::core::bindable_property<int>& max_length_property();
        static const maui::core::bindable_property<bool>& is_text_prediction_enabled_property();
        static const maui::core::bindable_property<bool>& is_spell_check_enabled_property();
        static const maui::core::bindable_property<int>& cursor_position_property();
        static const maui::core::bindable_property<int>& selection_length_property();
        static const maui::core::bindable_property<maui::core::return_type>& return_type_property();
        static const maui::core::bindable_property<maui::core::clear_button_visibility>&
        clear_button_visibility_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& horizontal_text_alignment_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& vertical_text_alignment_property();

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

        // ---- i_text_alignment ----
        [[nodiscard]] maui::core::text_alignment horizontal_text_alignment() const override
        {
            return horizontal_text_alignment_.get();
        }
        [[nodiscard]] maui::core::text_alignment vertical_text_alignment() const override
        {
            return vertical_text_alignment_.get();
        }

        // ---- i_entry ----
        [[nodiscard]] bool is_password() const override
        {
            return is_password_.get();
        }
        [[nodiscard]] maui::core::return_type return_type() const override
        {
            return return_type_.get();
        }
        [[nodiscard]] maui::core::clear_button_visibility clear_button_visibility() const override
        {
            return clear_button_visibility_.get();
        }

        // ---- i_text_input mutable cursor/selection (inbound: the handler writes the native position
        // back). Clamp the floor to 0 to mirror C#'s validateValue (>= 0). ----
        void set_cursor_position(int value) override
        {
            cursor_position_.set(value < 0 ? 0 : value);
        }
        void set_selection_length(int value) override
        {
            selection_length_.set(value < 0 ? 0 : value);
        }

        // ---- public setters (drive the handler via on_property_changed → update_value) ----
        // set_text applies max_length truncation (C# Entry semantics) before storing.
        void set_text(std::string value)
        {
            const int limit = max_length_.get();
            if (limit >= 0 && value.size() > static_cast<std::size_t>(limit))
            {
                value.resize(static_cast<std::size_t>(limit));
            }
            text_.set(std::move(value));
        }
        void set_placeholder(std::string value)
        {
            placeholder_.set(std::move(value));
        }
        void set_placeholder_color(maui::graphics::color value)
        {
            placeholder_color_.set(value);
        }
        void set_is_password(bool value)
        {
            is_password_.set(value);
        }
        void set_is_read_only(bool value)
        {
            is_read_only_.set(value);
        }
        // Setting a (non-negative) max_length truncates the current text to match, mirroring C#'s native
        // UpdateMaxLength re-trimming the existing value.
        void set_max_length(int value)
        {
            max_length_.set(value);
            if (value >= 0 && text_.get().size() > static_cast<std::size_t>(value))
            {
                std::string trimmed = text_.get();
                trimmed.resize(static_cast<std::size_t>(value));
                text_.set(std::move(trimmed));
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
        void set_clear_button_visibility(maui::core::clear_button_visibility value)
        {
            clear_button_visibility_.set(value);
        }

        // ---- i_entry inbound channel (called by the handler on native edits) ----
        void send_completed() override
        {
            if (!is_enabled())
            {
                return;
            }
            completed.raise();
        }
        void send_text_changed(std::string_view old_value, std::string_view new_value) override
        {
            text_changed.raise(std::string(old_value), std::string(new_value));
        }

        // ---- developer-facing events (the outbound channel) ----
        maui::core::event<> completed;
        maui::core::event<std::string, std::string> text_changed;

    private:
        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<std::string> placeholder_{*this, placeholder_property()};
        maui::core::property<maui::graphics::color> placeholder_color_{*this, placeholder_color_property()};
        maui::core::property<bool> is_password_{*this, is_password_property()};
        maui::core::property<bool> is_read_only_{*this, is_read_only_property()};
        maui::core::property<int> max_length_{*this, max_length_property()};
        maui::core::property<bool> is_text_prediction_enabled_{*this, is_text_prediction_enabled_property()};
        maui::core::property<bool> is_spell_check_enabled_{*this, is_spell_check_enabled_property()};
        maui::core::property<int> cursor_position_{*this, cursor_position_property()};
        maui::core::property<int> selection_length_{*this, selection_length_property()};
        maui::core::property<maui::core::return_type> return_type_{*this, return_type_property()};
        maui::core::property<maui::core::clear_button_visibility> clear_button_visibility_{
            *this, clear_button_visibility_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
        maui::core::property<maui::core::text_alignment> horizontal_text_alignment_{
            *this, horizontal_text_alignment_property()};
        maui::core::property<maui::core::text_alignment> vertical_text_alignment_{*this,
                                                                                  vertical_text_alignment_property()};
    };
} // namespace maui::controls
