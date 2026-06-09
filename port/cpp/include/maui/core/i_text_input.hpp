#pragma once
// maui::core::i_text_input  <=  Microsoft.Maui.ITextInput
//
// A view that takes keyboard input. Ported from src/Core/src/Core/ITextInput.cs
// (ITextInput : IText, IPlaceholder).
//
// IPlaceholder (src/Core/src/Core/IPlaceholder.cs) is folded in directly here — placeholder() /
// placeholder_color() — instead of a separate i_placeholder header, to keep the surface small; it
// can be split out later if another control reuses it.
//
// CursorPosition / SelectionLength are MUTABLE in C# (the field is the source of truth during a live
// edit, and the handler writes the *native* cursor/selection back onto the virtual view when the user
// moves it). They appear here as a getter plus an inbound setter (set_cursor_position /
// set_selection_length) the handler calls — the AppKit/headless analog of `entry.CursorPosition = …`
// inside the native selection-changed callback.
//
// Deferred (OUT OF SCOPE this cut, documented rather than stubbed): Keyboard (needs the Keyboard type
// subsystem). IsSpellCheckEnabled is included alongside IsTextPredictionEnabled (both map to the same
// kind of AppKit field-editor toggle).

#include <string_view>

#include "maui/core/i_text.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_text_input : public i_text
    {
    public:
        // ---- IPlaceholder ----
        [[nodiscard]] virtual std::string_view placeholder() const = 0;
        [[nodiscard]] virtual maui::graphics::color placeholder_color() const = 0;

        // ---- ITextInput value surface ----
        [[nodiscard]] virtual bool is_read_only() const = 0;
        [[nodiscard]] virtual int max_length() const = 0;
        [[nodiscard]] virtual bool is_text_prediction_enabled() const = 0;
        [[nodiscard]] virtual bool is_spell_check_enabled() const = 0;

        // ---- cursor / selection (mutable in C#) ----
        [[nodiscard]] virtual int cursor_position() const = 0;
        [[nodiscard]] virtual int selection_length() const = 0;
        // Inbound (the handler writes the native cursor/selection back onto the virtual view).
        virtual void set_cursor_position(int value) = 0;
        virtual void set_selection_length(int value) = 0;
    };
} // namespace maui::core
