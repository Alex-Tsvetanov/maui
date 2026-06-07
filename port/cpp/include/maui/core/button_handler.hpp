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
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties
    // (Visibility/Opacity/IsEnabled/AutomationId) onto it: the headless build keeps the base's mirrors,
    // the Apple build overrides update_* to push to the NSButton.
    struct button_platform : view_platform_base
    {
        button_platform() = default;
        // Destruction releases `native` and is therefore backend-defined (the headless build defaults
        // it; the Apple build CFReleases the retained NSButton). Non-copyable/non-movable: it is owned
        // solely by the handler's unique_ptr and never copied or moved.
        ~button_platform() override;
        button_platform(const button_platform&) = delete;
        button_platform(button_platform&&) = delete;
        button_platform& operator=(const button_platform&) = delete;
        button_platform& operator=(button_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple build pushes to `native` instead; these
        // let the headless tests observe that each mapper ran with the right value). The generic IView
        // mirrors (hidden/alpha/enabled/automation_id) come from view_platform_base.
        std::string title;
        maui::graphics::color text_color;
        font text_font;
        double character_spacing = 0;
        thickness padding;
        maui::graphics::color stroke_color;
        double stroke_thickness = 0;
        int corner_radius = 0;
        move_only_function<void()> on_click;
        move_only_function<void()> on_press;
        move_only_function<void()> on_release;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSButton (defined in
        // src/platform/apple/button_handler.mm). The headless build omits these and keeps the
        // view_platform_base mirrors — the class layout is identical (same four virtual slots), and a
        // given build only ever sees one backend's definition, so there is no ODR mismatch.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
#endif
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

        // Property map functions (platform recipe), each pushing one virtual-view property onto the
        // platform view. Text/appearance are keyed by i_text_button (C#'s TextButtonMapper<ITextButton>);
        // padding + stroke are keyed by i_button (the button's own mapper).
        static void map_text(button_handler& handler, i_text_button& view);
        static void map_text_color(button_handler& handler, i_text_button& view);
        static void map_font(button_handler& handler, i_text_button& view);
        static void map_character_spacing(button_handler& handler, i_text_button& view);
        static void map_padding(button_handler& handler, i_button& view);
        static void map_stroke_color(button_handler& handler, i_button& view);
        static void map_stroke_thickness(button_handler& handler, i_button& view);
        static void map_corner_radius(button_handler& handler, i_button& view);
    };
} // namespace maui::core
