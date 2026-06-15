#pragma once
// maui::controls::editor  <=  Microsoft.Maui.Controls.Editor
//
// A multi-line text editor. Ported from Editor.cs + InputView.cs. Same API shape as entry: bare-noun
// interface getters + method accessors, each backed by a private property<T> whose change flows through
// view::on_property_changed to the handler (virtual→native). The reverse direction is the send_*
// methods: on a native edit the handler calls them and the control raises its public events.
//
// text_changed semantics (InputView.OnTextChanged → TextChanged): in C# BOTH a programmatic Text set and
// a native edit funnel through the Text property's propertyChanged, so set_text raises text_changed with
// (old, new) — the EditorTests.EditorTextChangedEventArgs oracle. A native edit arrives through
// send_text_changed (the native view is the source of truth during a live edit, so it raises directly
// and does NOT re-push through the property store). Both paths run the AutoSize hook: when auto_size is
// text_changes a text change invalidates measure (Editor.OnTextChanged → InvalidateMeasure).
//
// completed: send_completed() raises `completed` unconditionally — Editor.SendCompleted has NO IsEnabled
// gate (unlike Entry.SendCompleted; the difference is preserved).
//
// auto_size: control-level only (no mapper key — C#'s AutoSizeProperty drives measure invalidation, not
// a native push). Default disabled. Editor.MeasureOverride's previous-constraint caching is NOT ported
// (it leans on the Controls legacy measure pipeline); the handler's get_desired_size is authoritative.
//
// max_length: set_text truncates to max_length, and lowering max_length re-trims the stored text — the
// same control-side enforcement entry applies (C# enforces natively via UpdateMaxLength).
//
// Deferred (OUT OF SCOPE this cut, documented not stubbed): FontAutoScaling, TextTransform.

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/editor_auto_size_option.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_editor.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/property.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class editor : public view<maui::core::i_editor>
    {
    public:
        // Declare the style TargetType so an implicit / class style targeting `editor` matches this control.
        editor()
        {
            this->set_style_target_type<editor>();
        }

        // Shared bindable-property descriptors (one instance per type, like Editor.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<std::string>& placeholder_property();
        static const maui::core::bindable_property<maui::graphics::color>& placeholder_color_property();
        static const maui::core::bindable_property<bool>& is_read_only_property();
        static const maui::core::bindable_property<int>& max_length_property();
        static const maui::core::bindable_property<bool>& is_text_prediction_enabled_property();
        static const maui::core::bindable_property<bool>& is_spell_check_enabled_property();
        static const maui::core::bindable_property<int>& cursor_position_property();
        static const maui::core::bindable_property<int>& selection_length_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& horizontal_text_alignment_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& vertical_text_alignment_property();
        static const maui::core::bindable_property<editor_auto_size_option>& auto_size_property();
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

        // ---- i_text_alignment (Editor defaults: Start / Start — its own VerticalTextAlignmentProperty) ----
        [[nodiscard]] maui::core::text_alignment horizontal_text_alignment() const override
        {
            return horizontal_text_alignment_.get();
        }
        [[nodiscard]] maui::core::text_alignment vertical_text_alignment() const override
        {
            return vertical_text_alignment_.get();
        }

        // ---- AutoSize (Editor.AutoSize — control-level, drives measure invalidation only) ----
        [[nodiscard]] editor_auto_size_option auto_size() const
        {
            return auto_size_.get();
        }
        void set_auto_size(editor_auto_size_option value)
        {
            auto_size_.set(value);
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
        // set_text applies max_length truncation, then raises text_changed with (old, new) when the value
        // actually changed (InputView's TextProperty propertyChanged → OnTextChanged).
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
                on_text_changed_core(old, std::string(text_.get()));
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
        void set_is_read_only(bool value)
        {
            is_read_only_.set(value);
        }
        // Setting a (non-negative) max_length truncates the current text to match (entry's convention for
        // C#'s native UpdateMaxLength re-trim). The truncation routes through set_text so text_changed +
        // the AutoSize hook fire for the trimmed value too.
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
        void set_keyboard(maui::core::keyboard value)
        {
            keyboard_.set(value);
        }

        // ---- i_editor inbound channel (called by the handler on native edits) ----
        // Editor.SendCompleted has no IsEnabled gate (unlike Entry) — preserved faithfully.
        void send_completed() override
        {
            completed.raise();
        }
        void send_text_changed(std::string_view old_value, std::string_view new_value) override
        {
            on_text_changed_core(std::string(old_value), std::string(new_value));
        }

        // ---- developer-facing events (the outbound channel) ----
        maui::core::event<> completed;
        maui::core::event<std::string, std::string> text_changed;

    private:
        // The shared Text-changed tail (InputView.OnTextChanged + Editor.OnTextChanged): raise the event,
        // then invalidate measure when AutoSize is TextChanges.
        void on_text_changed_core(const std::string& old_value, const std::string& new_value)
        {
            text_changed.raise(old_value, new_value);
            if (auto_size_.get() == editor_auto_size_option::text_changes)
            {
                this->invalidate_measure();
            }
        }

        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<std::string> placeholder_{*this, placeholder_property()};
        maui::core::property<maui::graphics::color> placeholder_color_{*this, placeholder_color_property()};
        maui::core::property<bool> is_read_only_{*this, is_read_only_property()};
        maui::core::property<int> max_length_{*this, max_length_property()};
        maui::core::property<bool> is_text_prediction_enabled_{*this, is_text_prediction_enabled_property()};
        maui::core::property<bool> is_spell_check_enabled_{*this, is_spell_check_enabled_property()};
        maui::core::property<int> cursor_position_{*this, cursor_position_property()};
        maui::core::property<int> selection_length_{*this, selection_length_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
        maui::core::property<maui::core::text_alignment> horizontal_text_alignment_{
            *this, horizontal_text_alignment_property()};
        maui::core::property<maui::core::text_alignment> vertical_text_alignment_{*this,
                                                                                  vertical_text_alignment_property()};
        maui::core::property<editor_auto_size_option> auto_size_{*this, auto_size_property()};
        maui::core::property<maui::core::keyboard> keyboard_{*this, keyboard_property()};
    };
} // namespace maui::controls
