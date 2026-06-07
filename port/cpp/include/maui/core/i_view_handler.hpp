#pragma once
// maui::core::i_view_handler  <=  Microsoft.Maui.IViewHandler
//
// The handler contract for views (IView). Extends i_element_handler with container management, a typed
// (covariant) virtual-view accessor, and the platform measure/arrange seam. Ported from
// src/Core/src/Handlers/IViewHandler.cs.

#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct view_platform_base; // the generic-IView platform-view face (view_platform_base.hpp)

    class i_view_handler : public i_element_handler
    {
    public:
        ~i_view_handler() override = default;

        [[nodiscard]] virtual bool has_container() const = 0;
        virtual void set_has_container(bool value) = 0;
        [[nodiscard]] virtual void* container_view() const = 0;

        // The platform view's view_platform_base face, used by the shared view_mapper to push the
        // generic IView properties — or null when the platform view does not derive view_platform_base
        // (then those maps are no-ops). The CRTP view_handler resolves this with `if constexpr`.
        [[nodiscard]] virtual view_platform_base* platform_base() const = 0;

        // C#'s `new IView? VirtualView`: a covariant narrowing of i_element_handler::virtual_view()
        // (i_view derives i_element). Concrete CRTP handlers narrow it further to their Virtual type.
        [[nodiscard]] i_view* virtual_view() const override = 0;

        // C# GetDesiredSize / PlatformArrange: the platform measure + arrange seam (abstract — each
        // concrete handler implements them against its platform view).
        [[nodiscard]] virtual maui::graphics::size get_desired_size(double width_constraint,
                                                                    double height_constraint) const = 0;
        virtual void platform_arrange(const maui::graphics::rect& frame) = 0;

    protected:
        i_view_handler() = default;
        i_view_handler(const i_view_handler&) = default;
        i_view_handler(i_view_handler&&) = default;
        i_view_handler& operator=(const i_view_handler&) = default;
        i_view_handler& operator=(i_view_handler&&) = default;
    };
} // namespace maui::core
