#pragma once
// maui::controls::table_section_base  <=  Microsoft.Maui.Controls.TableSectionBase
//
// The abstract base for a table_view section (and for the table_root): a bindable_object carrying the
// section header Title + TextColor. Ported from src/Controls/src/Core/TableView/TableSectionBase.cs.
//
// Surface: Title / TextColor (bindable). The string-title constructor is modeled by a set_title-style
// helper on the concrete subclasses (table_section / table_root each take an optional title). Color
// collapse (port convention): nullable Color → color value.

#include <string>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class table_section_base : public maui::core::bindable_object
    {
    public:
        ~table_section_base() override = default;
        table_section_base(const table_section_base&) = delete;
        table_section_base(table_section_base&&) = delete;
        table_section_base& operator=(const table_section_base&) = delete;
        table_section_base& operator=(table_section_base&&) = delete;

        // Shared bindable-property descriptors (one instance per type, like TableSectionBase.*Property).
        static const maui::core::bindable_property<std::string>& title_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();

        [[nodiscard]] std::string title() const
        {
            return title_.get();
        }
        void set_title(std::string value)
        {
            title_.set(std::move(value));
        }
        [[nodiscard]] maui::graphics::color text_color() const
        {
            return text_color_.get();
        }
        void set_text_color(maui::graphics::color value)
        {
            text_color_.set(value);
        }

    protected:
        table_section_base() = default;
        explicit table_section_base(std::string title)
        {
            title_.set(std::move(title));
        }

    private:
        maui::core::property<std::string> title_{*this, title_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
    };
} // namespace maui::controls
