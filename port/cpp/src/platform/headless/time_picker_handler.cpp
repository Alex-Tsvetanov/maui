// time_picker_handler — headless platform recipe. A testable stand-in for the native time field: the
// maps run the C# TimePickerExtensions.UpdateTime algorithm against the time_picker_platform mirrors
// (the wheel value + the formatted display text), and on_done is the Done-accessory commit
// (SetVirtualViewTime — hours+minutes only, seconds dropped) tests invoke directly to simulate a
// native pick. The iOS backend (src/platform/ios/time_picker_handler.mm) is the real-native twin;
// AppKit translates to a time-only NSDatePicker.

#include "maui/core/time_picker_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/date_time.hpp"
#include "maui/core/i_time_picker.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    namespace
    {
        // TimePickerExtensions.UpdateTime: push the virtual time onto the wheel (a null Time falls
        // back to zero), then render the display text — null Time shows empty; the format renders
        // through TimeExtensions.ToFormattedString (empty falls back to "t"). The C# per-format
        // culture pick (en-US for t/h, de-DE for H) collapses into the port's invariant/en-US
        // rendering (see date_time.hpp).
        void update_time(time_picker_handler& handler, i_time_picker& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            const auto time = view.time();
            platform->time = time.value_or(time_span{});
            platform->text = time.has_value() ? format_time_span(*time, view.format()) : std::string{};
        }
    } // namespace

    // Headless has no native view in the `native` slot, so destruction is trivial.
    time_picker_platform::~time_picker_platform() = default;

    std::unique_ptr<time_picker_platform> time_picker_handler::create_platform_view()
    {
        return std::make_unique<time_picker_platform>();
    }

    void time_picker_handler::on_connect_handler(time_picker_platform& platform)
    {
        // The Done-accessory commit (MauiTimePicker's dateSelected → SetVirtualViewTime): hours and
        // minutes only — C# builds `new TimeSpan(datetime.Hour, datetime.Minute, 0)`.
        platform.on_done = [this] {
            auto* view = virtual_view();
            auto* typed = typed_platform_view();
            if (view == nullptr || typed == nullptr)
            {
                return;
            }
            view->set_time(time_span(typed->time.hours(), typed->time.minutes(), 0));
        };
    }

    void time_picker_handler::on_disconnect_handler(time_picker_platform& platform)
    {
        platform.on_done = nullptr;
    }

    void time_picker_handler::map_format(time_picker_handler& handler, i_time_picker& view)
    {
        update_time(handler, view); // UpdateFormat routes into UpdateTime (re-render the text)
    }

    void time_picker_handler::map_time(time_picker_handler& handler, i_time_picker& view)
    {
        update_time(handler, view);
    }

    void time_picker_handler::map_text_color(time_picker_handler& handler, i_time_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void time_picker_handler::map_font(time_picker_handler& handler, i_time_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void time_picker_handler::map_character_spacing(time_picker_handler& handler, i_time_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    maui::graphics::size time_picker_handler::get_desired_size(double width_constraint,
                                                               double /*height_constraint*/) const
    {
        // Headless placeholder metric (no real text layout): a single-line field ~150pt wide by
        // default, clamped to a finite width constraint, fixed line height (the entry convention).
        double width = 150.0;
        if (width_constraint > 0 && width_constraint < width)
        {
            width = width_constraint;
        }
        return {width, 22.0};
    }

    void time_picker_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
