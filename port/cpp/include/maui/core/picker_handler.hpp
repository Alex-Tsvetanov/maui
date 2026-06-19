#pragma once
// maui::core::picker_handler  <=  Microsoft.Maui.Handlers.PickerHandler
//
// The handler for the item picker. Items/selection/title/appearance flow virtual→native through the
// property mapper (items travel through the i_item_delegate face — get_count/get_item — exactly like
// the C# platform recipes); a native row pick flows native→virtual via i_picker::set_selected_index
// (the control stores it at from_handler specificity and raises selected_index_changed). Ported from
// PickerHandler.cs (cross-platform) + PickerHandler.iOS.cs / Platform/iOS/MauiPicker.cs +
// PickerExtensions.cs (the UITextField-whose-inputView-is-a-UIPickerView recipe — replicated 1:1 on
// the ios backend; the AppKit backend translates it idiomatically to NSPopUpButton, deviations
// documented in the .mm).
//
// Partial-class split (PROFILE §5): mapper tables + ctor here/cpp; the platform recipe per backend
// under src/platform/<backend>/picker_handler.{cpp,mm}.
//
// picker_platform mirrors every mapped property for the headless backend (`native` holds the real
// backend view elsewhere). `text` is the MauiPicker.Text analog — the display string the recipe
// derives from the selection (GetItem(selectedIndex), or empty + the title placeholder at -1).
// `on_done` is the inbound channel: the headless stand-in for the Done-accessory tap that commits the
// native wheel row (FinishSelectItem); tests invoke it directly with the picked row.

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_picker.hpp"
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
    struct picker_platform : view_platform_base
    {
        picker_platform() = default;
        ~picker_platform() override; // backend-defined: releases the retained native view on Apple/iOS
        picker_platform(const picker_platform&) = delete;
        picker_platform(picker_platform&&) = delete;
        picker_platform& operator=(const picker_platform&) = delete;
        picker_platform& operator=(picker_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds write to `native` instead).
        std::vector<std::string> items;
        int selected_index = -1;
        std::string text; // the displayed selection (MauiPicker.Text)
        std::string title;
        maui::graphics::color title_color;
        maui::graphics::color text_color;
        font text_font;
        double character_spacing = 0;
        text_alignment horizontal_alignment = text_alignment::start;
        text_alignment vertical_alignment = text_alignment::center;

        // Inbound channel (wired by the platform partial; headless tests invoke it directly): commit
        // the native wheel's pending row — the Done-tap / popup-action analog (FinishSelectItem).
        move_only_function<void(int row)> on_done;

#ifdef MAUI_PLATFORM_APPLE
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // BackgroundColor IS pushed to the UITextField (it respects backgroundColor directly, unlike a
        // UIButton): the MauiPicker is a plain UIView, so the shared apply_background paints its layer.
        void update_background(const maui::graphics::paint* value) override;
#endif
    };

    class picker_handler : public view_handler<picker_handler, i_picker, picker_platform>
    {
    public:
        picker_handler();

        static property_mapper<i_picker, picker_handler>& mapper();
        static command_mapper<i_picker, picker_handler>& command_mapper();

        static std::unique_ptr<picker_platform> create_platform_view();
        void on_connect_handler(picker_platform& platform);
        static void on_disconnect_handler(picker_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe). map_items is the C# Reload/UpdatePicker; the
        // selection map shares its body (both route through the UpdatePicker(picker, index) helper).
        static void map_items(picker_handler& handler, i_picker& view);
        static void map_selected_index(picker_handler& handler, i_picker& view);
        static void map_title(picker_handler& handler, i_picker& view);
        static void map_title_color(picker_handler& handler, i_picker& view);
        static void map_text_color(picker_handler& handler, i_picker& view);
        static void map_font(picker_handler& handler, i_picker& view);
        static void map_character_spacing(picker_handler& handler, i_picker& view);
        static void map_horizontal_text_alignment(picker_handler& handler, i_picker& view);
        static void map_vertical_text_alignment(picker_handler& handler, i_picker& view);
        // PickerHandler.MapIsOpen: become first responder when IsOpen, else resign (focus the native
        // field opens its inputView wheel; resigning dismisses it).
        static void map_is_open(picker_handler& handler, i_picker& view);
    };
} // namespace maui::core
