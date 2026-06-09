#pragma once
// maui::core::label_handler  <=  Microsoft.Maui.Handlers.LabelHandler
//
// The handler for a text label — the second concrete control, proving the handler recipe generalizes.
// Display-only: no inbound event channel (unlike button), so the mapper is a single table keyed on
// i_label (which already exposes the text/appearance/alignment via its bases — no chaining needed).
// Ported from LabelHandler.cs (+ LabelHandler.iOS.cs, translated to AppKit's NSTextField).
//
// Same partial-class split + single cross-platform label_platform struct as button_handler.

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_label.hpp"
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
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto
    // it (headless keeps the base mirrors; Apple overrides update_* to push to the NSTextField).
    struct label_platform : view_platform_base
    {
        label_platform() = default;
        ~label_platform() override; // backend-defined: releases the retained native label on Apple
        label_platform(const label_platform&) = delete;
        label_platform(label_platform&&) = delete;
        label_platform& operator=(const label_platform&) = delete;
        label_platform& operator=(label_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of the mapped properties (the Apple build writes to `native` instead). The
        // generic IView mirrors (hidden/alpha/enabled/automation_id) come from view_platform_base.
        std::string text;
        maui::graphics::color text_color;
        font text_font;
        text_alignment horizontal_alignment = text_alignment::start;
        text_alignment vertical_alignment = text_alignment::start; // C# Label default Start
        double character_spacing = 0;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSTextField (defined in
        // src/platform/apple/label_handler.mm). Omitted on headless, which keeps the base mirrors; the
        // class layout is identical and a build only ever sees one backend, so there is no ODR mismatch.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
#endif
    };

    class label_handler : public view_handler<label_handler, i_label, label_platform>
    {
    public:
        label_handler();

        static property_mapper<i_label, label_handler>& mapper();
        static command_mapper<i_label, label_handler>& command_mapper();

        static std::unique_ptr<label_platform> create_platform_view();

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe).
        static void map_text(label_handler& handler, i_label& view);
        static void map_text_color(label_handler& handler, i_label& view);
        static void map_font(label_handler& handler, i_label& view);
        static void map_horizontal_text_alignment(label_handler& handler, i_label& view);
        static void map_vertical_text_alignment(label_handler& handler, i_label& view);
        static void map_character_spacing(label_handler& handler, i_label& view);
    };
} // namespace maui::core
