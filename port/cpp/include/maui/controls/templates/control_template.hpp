#pragma once
// maui::controls::control_template  <=  Microsoft.Maui.Controls.ControlTemplate
//
// The appearance of a templated control: a bare element_template (no Values/Bindings staging — that
// is data_template's). Created either from a loader function (the C# Func<object> ctor, null →
// std::invalid_argument) or via the reflection-free Type-ctor stand-in of<TControl>(). The created
// content must be a View — template_utilities::on_control_template_changed enforces it (the C#
// NotSupportedException), not the template itself.

#include <memory>
#include <utility>

#include "maui/controls/templates/element_template.hpp"
#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    class control_template : public element_template
    {
    public:
        // ControlTemplate() — an empty template (create_content yields the label fallback).
        control_template() = default;
        // ControlTemplate(Func<object>) — throws std::invalid_argument on null (ArgumentNullException).
        explicit control_template(loader create_template) : element_template(std::move(create_template))
        {
        }

        // ControlTemplate(Type) — the reflection-free stand-in: load via make_shared<TControl>.
        template <class TControl> [[nodiscard]] static std::shared_ptr<control_template> of()
        {
            return std::shared_ptr<control_template>(new control_template(
                [] { return std::static_pointer_cast<maui::core::bindable_object>(std::make_shared<TControl>()); },
                /*type_activated=*/true));
        }

    private:
        // The type-activated form (can_recycle = true), reached through of<TControl>().
        control_template(loader create_template, bool type_activated)
            : element_template(std::move(create_template), type_activated)
        {
        }
    };
} // namespace maui::controls
