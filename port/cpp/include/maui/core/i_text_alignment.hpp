#pragma once
// maui::core::i_text_alignment  <=  Microsoft.Maui.ITextAlignment
// The horizontal/vertical text-alignment contract. Ported from src/Core/src/Core/ITextAlignment.cs.

#include "maui/core/text_alignment.hpp"

namespace maui::core
{
    class i_text_alignment
    {
    public:
        virtual ~i_text_alignment() = default;

        [[nodiscard]] virtual text_alignment horizontal_text_alignment() const = 0;
        [[nodiscard]] virtual text_alignment vertical_text_alignment() const = 0;

    protected:
        i_text_alignment() = default;
        i_text_alignment(const i_text_alignment&) = default;
        i_text_alignment(i_text_alignment&&) = default;
        i_text_alignment& operator=(const i_text_alignment&) = default;
        i_text_alignment& operator=(i_text_alignment&&) = default;
    };
} // namespace maui::core
