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
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/label_run.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
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
        maui::core::text_decorations decorations = maui::core::text_decorations::none;
        // Headless mirror of the resolved attributed runs (Label.FormattedText). Empty = plain text path.
        // The Apple/iOS builds turn these into an NSAttributedString instead; the headless build keeps the
        // run list so tests can assert per-span attributes flowed to the platform mirror.
        std::vector<maui::core::label_run> formatted_text_runs;

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
        // Accessibility metadata + the input-transparent flag pushed to the NSTextField (M5d native a11y /
        // hit-test): semantics → accessibilityLabel/Help/heading role, input_transparent → -hitTest: gate.
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (M6 fan-out): push the four fundamental IView properties to the UILabel (defined
        // in src/platform/ios/label_handler.mm). The remaining generic-IView pushes — transform /
        // flow_direction / background / shadow / clip / semantics / input_transparent — keep the
        // view_platform_base mirrors until the shared ios view/visual/semantics op helpers land (the
        // coordinator's retrofit; see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
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
        // LabelHandler.MapTextDecorations → LabelExtensions.UpdateTextDecorations (underline /
        // strikethrough on the attributed text; headless mirrors the flags).
        static void map_text_decorations(label_handler& handler, i_label& view);
        // The port's gap-closure G1 map: Label.FormattedText → LabelExtensions.UpdateText's FormattedText
        // branch. Non-empty runs build the native attributed string (NSAttributedString on apple/ios; the
        // headless run mirror); empty runs revert to the plain text path (clearing the attributed string).
        static void map_formatted_text(label_handler& handler, i_label& view);
    };
} // namespace maui::core
