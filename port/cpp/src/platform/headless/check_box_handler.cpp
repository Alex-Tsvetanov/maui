// check_box_handler — headless platform recipe. A testable stand-in for a native check box: the mapped
// properties mirror into check_box_platform, and `is_checked` doubles as the native checked state — a
// test simulates a user tap by flipping it and invoking on_checked_changed (the MauiCheckBox
// .CheckedChanged analog), which writes the value back through i_check_box::send_is_checked exactly
// like C#'s CheckBoxHandler.OnCheckedChanged. The Apple/iOS .mm partials are the real twins.

#include "maui/core/check_box_handler.hpp"

#include <memory>

#include "maui/core/i_check_box.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    check_box_platform::~check_box_platform() = default;

    std::unique_ptr<check_box_platform> check_box_handler::create_platform_view()
    {
        return std::make_unique<check_box_platform>();
    }

    void check_box_handler::on_connect_handler(check_box_platform& platform)
    {
        // CheckBoxHandler.OnCheckedChanged: write the native state back to the virtual view.
        platform.on_checked_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr && view->is_checked() != platform_view->is_checked)
            {
                view->send_is_checked(platform_view->is_checked);
            }
        };
    }

    void check_box_handler::on_disconnect_handler(check_box_platform& platform)
    {
        platform.on_checked_changed = nullptr;
    }

    void check_box_handler::map_is_checked(check_box_handler& handler, i_check_box& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_checked = view.is_checked();
        }
    }

    void check_box_handler::map_foreground(check_box_handler& handler, i_check_box& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->foreground = view.foreground();
        }
    }

    maui::graphics::size check_box_handler::get_desired_size(double /*width_constraint*/,
                                                             double /*height_constraint*/) const
    {
        // Headless placeholder metric: the iOS handler's MinimumSize square (44x44).
        return {44.0, 44.0};
    }

    void check_box_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
