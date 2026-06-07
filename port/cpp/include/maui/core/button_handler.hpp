#pragma once
// maui::core::button_handler  <=  Microsoft.Maui.Handlers.ButtonHandler
//
// The first concrete handler — the Rosetta Stone (PROJECT.md M2). It maps the cross-platform i_button
// virtual view to a native button: Text flows virtual→native through the property mapper, and a native
// tap flows native→virtual by calling i_button::send_clicked() (which the control turns into its
// `clicked` event). Ported from ButtonHandler.cs (cross-platform) + ButtonHandler.iOS.cs (the platform
// recipe, translated to each backend).
//
// Partial-class split (PROFILE §5): the mapper TABLES and ctor are cross-platform (button_handler.cpp);
// the platform recipe — create_platform_view / connect / disconnect / map_text / measure — is defined
// per backend under src/platform/<backend>/button_handler.{cpp,mm}. Only one backend is linked.
//
// button_platform is the managed platform view. It is a single cross-platform struct (so the CRTP
// Platform type stays complete everywhere — no incomplete-type pimpl dance): the `native` slot holds
// the real backend view (an NSButton* on Apple, retained in the .mm; unused headless), `title` is the
// headless text mirror, and the callbacks are the inbound event hooks the platform partial wires up.

#include <memory>
#include <string>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct button_platform
    {
        void* native = nullptr;
        std::string title;
        bool enabled = true;
        move_only_function<void()> on_click;
        move_only_function<void()> on_press;
        move_only_function<void()> on_release;
    };

    class button_handler : public view_handler<button_handler, i_button, button_platform>
    {
    public:
        button_handler();

        // Shared mapper tables (cross-platform — defined in button_handler.cpp). `mapper` chains the
        // text mapper, mirroring ButtonHandler.Mapper chaining TextButtonMapper.
        static property_mapper<i_button, button_handler>& mapper();
        static property_mapper<i_text_button, button_handler>& text_mapper();
        static command_mapper<i_button, button_handler>& command_mapper();

        // Platform recipe (defined per backend: src/platform/<backend>/button_handler.{cpp,mm}).
        // create + disconnect need no handler state (static); connect captures `this` to route the
        // native control's events back to the virtual view.
        static std::unique_ptr<button_platform> create_platform_view();
        void on_connect_handler(button_platform& platform);
        static void on_disconnect_handler(button_platform& platform);

        // i_view_handler measure/arrange seam (platform-specific sizing).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map function (platform recipe): pushes the virtual view's text onto the platform
        // view. Keyed by i_text_button (C#'s TextButtonMapper<ITextButton>; ButtonHandler.MapText).
        static void map_text(button_handler& handler, i_text_button& view);
    };
} // namespace maui::core
