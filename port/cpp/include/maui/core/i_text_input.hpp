#pragma once
// maui::core::i_text_input  <=  Microsoft.Maui.ITextInput
//
// A view that takes keyboard input. Ported from src/Core/src/Core/ITextInput.cs
// (ITextInput : IText, IPlaceholder). This first cut keeps to the simple VALUE properties an editable
// native control needs: placeholder (+ its color), is_read_only, and max_length.
//
// IPlaceholder (src/Core/src/Core/IPlaceholder.cs) is folded in directly here — placeholder() /
// placeholder_color() — instead of a separate i_placeholder header, to keep the M4b surface small; it
// can be split out later if another control reuses it.
//
// Deferred (OUT OF SCOPE this cut, documented rather than stubbed): the C# ITextInput also exposes the
// cursor/selection pair CursorPosition/SelectionLength (mutable — needs a native first responder),
// Keyboard (needs the Keyboard type subsystem), and IsTextPredictionEnabled / IsSpellCheckEnabled.

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
    };
} // namespace maui::core
