#pragma once
// maui::core::entry_handler  <=  Microsoft.Maui.Handlers.EntryHandler
//
// The handler for a single-line text entry — the first editable native control and the first INBOUND-text
// channel. Text/placeholder/secure/read-only/alignment/appearance flow virtual→native through the property
// mapper; a native edit flows native→virtual by calling i_entry::send_text_changed(old, new) (the control
// turns it into its `text_changed` event) and i_entry::send_completed() on end-of-edit. Ported from
// EntryHandler.cs (cross-platform) + EntryHandler.iOS.cs (the UITextField recipe, translated to AppKit's
// editable NSTextField / NSSecureTextField).
//
// Partial-class split (PROFILE §5): the mapper TABLES + ctor are cross-platform (entry_handler.cpp); the
// platform recipe — create/connect/disconnect/map_*/measure — is per backend under
// src/platform/<backend>/entry_handler.{cpp,mm}. Only one backend is linked.
//
// entry_platform is a single cross-platform struct (so the CRTP Platform type stays complete everywhere):
// `native` holds the real backend view (an editable NSTextField* on Apple, retained in the .mm; unused
// headless), the value fields mirror every mapped property (the headless tests observe them; the Apple
// build pushes to `native` instead), `last_known_text` lets the inbound edit supply the *old* value, and
// the move_only_function hooks are the inbound channel the platform partial wires up (the headless test
// invokes them directly to simulate a native edit / end-of-edit).
//
// entry_platform derives view_platform_base (the shared ViewMapper face) so the generic IView properties
// (Visibility/Opacity/IsEnabled/AutomationId) map onto the field too (Apple overrides update_*; headless
// keeps the base mirrors).

#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct entry_platform : view_platform_base
    {
        entry_platform() = default;
        ~entry_platform() override; // backend-defined: releases the retained native field on Apple
        entry_platform(const entry_platform&) = delete;
        entry_platform(entry_platform&&) = delete;
        entry_platform& operator=(const entry_platform&) = delete;
        entry_platform& operator=(entry_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple build writes to `native` instead).
        std::string text;
        std::string placeholder;
        bool is_password = false;
        bool is_read_only = false;
        int max_length = std::numeric_limits<int>::max(); // C# default: no effective cap
        maui::graphics::color text_color;
        maui::graphics::color placeholder_color;
        font text_font;
        double character_spacing = 0;
        text_alignment horizontal_alignment = text_alignment::start;
        text_alignment vertical_alignment = text_alignment::center;

        // The last text the entry is known to hold, so an inbound edit can report the *old* value.
        std::string last_known_text;

        // Inbound channel hooks (wired by the platform partial; headless tests invoke them directly).
        move_only_function<void(const std::string& old_value, const std::string& new_value)> on_text_changed;
        move_only_function<void()> on_completed;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSTextField (defined in
        // src/platform/apple/entry_handler.mm). Omitted on headless, which keeps the base mirrors.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class entry_handler : public view_handler<entry_handler, i_entry, entry_platform>
    {
    public:
        entry_handler();

        static property_mapper<i_entry, entry_handler>& mapper();
        static command_mapper<i_entry, entry_handler>& command_mapper();

        // Platform recipe (per backend). create + disconnect need no handler state (static); connect
        // captures `this` to route the native field's edits back to the virtual view.
        static std::unique_ptr<entry_platform> create_platform_view();
        void on_connect_handler(entry_platform& platform);
        static void on_disconnect_handler(entry_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe), each pushing one virtual-view property to the field.
        static void map_text(entry_handler& handler, i_entry& view);
        static void map_placeholder(entry_handler& handler, i_entry& view);
        static void map_placeholder_color(entry_handler& handler, i_entry& view);
        static void map_is_password(entry_handler& handler, i_entry& view);
        static void map_is_read_only(entry_handler& handler, i_entry& view);
        static void map_max_length(entry_handler& handler, i_entry& view);
        static void map_text_color(entry_handler& handler, i_entry& view);
        static void map_font(entry_handler& handler, i_entry& view);
        static void map_character_spacing(entry_handler& handler, i_entry& view);
        static void map_horizontal_text_alignment(entry_handler& handler, i_entry& view);
        static void map_vertical_text_alignment(entry_handler& handler, i_entry& view);
    };
} // namespace maui::core
