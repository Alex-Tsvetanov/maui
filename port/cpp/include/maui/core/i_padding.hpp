#pragma once
// maui::core::i_padding  <=  Microsoft.Maui.IPadding
//
// The space between a control's outer edge and its content. Ported from src/Core/src/Core/IPadding.cs.

#include "maui/core/thickness.hpp"

namespace maui::core
{
    class i_padding
    {
    public:
        virtual ~i_padding() = default;

        [[nodiscard]] virtual thickness padding() const = 0;

    protected:
        i_padding() = default;
        i_padding(const i_padding&) = default;
        i_padding(i_padding&&) = default;
        i_padding& operator=(const i_padding&) = default;
        i_padding& operator=(i_padding&&) = default;
    };
} // namespace maui::core
