#pragma once
// maui::core::i_text  <=  Microsoft.Maui.IText
// A text-bearing view contract. Ported from src/Core/src/Core/IText.cs. text() returns a string_view
// over the implementation's stored text (valid until the text changes). Inherits i_text_style.

#include <string_view>

#include "maui/core/i_text_style.hpp"

namespace maui::core
{
    class i_text : public i_text_style
    {
    public:
        [[nodiscard]] virtual std::string_view text() const = 0;
    };
} // namespace maui::core
