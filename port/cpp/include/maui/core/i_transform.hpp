#pragma once
// maui::core::i_transform  <=  Microsoft.Maui.ITransform
// The render-transform contract (a base of i_view). Ported from src/Core/src/Core/ITransform.cs.

namespace maui::core
{
    class i_transform
    {
    public:
        virtual ~i_transform() = default;

        [[nodiscard]] virtual double translation_x() const = 0;
        [[nodiscard]] virtual double translation_y() const = 0;
        [[nodiscard]] virtual double scale() const = 0;
        [[nodiscard]] virtual double scale_x() const = 0;
        [[nodiscard]] virtual double scale_y() const = 0;
        [[nodiscard]] virtual double rotation() const = 0;
        [[nodiscard]] virtual double rotation_x() const = 0;
        [[nodiscard]] virtual double rotation_y() const = 0;
        [[nodiscard]] virtual double anchor_x() const = 0;
        [[nodiscard]] virtual double anchor_y() const = 0;

    protected:
        i_transform() = default;
        i_transform(const i_transform&) = default;
        i_transform(i_transform&&) = default;
        i_transform& operator=(const i_transform&) = default;
        i_transform& operator=(i_transform&&) = default;
    };
} // namespace maui::core
