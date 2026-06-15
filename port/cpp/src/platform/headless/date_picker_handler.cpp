// date_picker_handler — headless platform recipe. A testable stand-in for the native date field: the
// maps run the C# DatePickerExtensions.UpdateDate algorithm against the date_picker_platform mirrors
// (the dialog date + the formatted display text), and on_done is the Done-accessory commit
// (OnDoneClicked → SetVirtualViewDate) tests invoke directly to simulate a native pick. The iOS
// backend (src/platform/ios/date_picker_handler.mm) is the real-native twin; AppKit translates to
// NSDatePicker.

#include "maui/core/date_picker_handler.hpp"

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/date_time.hpp"
#include "maui/core/i_date_picker.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    namespace
    {
        // DatePickerExtensions.UpdateDate: push the virtual date onto the dialog (a null Date falls
        // back to Today), then render the display text — null Date shows empty; "d"/"D"/empty route
        // through the standard patterns, anything else is a custom DateTime.ToString pattern. The
        // port renders in the invariant/en-US culture (see date_time.hpp).
        void update_date(date_picker_handler& handler, i_date_picker& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            const auto date = view.date();
            platform->date = date.value_or(date_time::today());
            if (!date.has_value())
            {
                platform->text.clear();
                return;
            }
            const std::string_view format = view.format();
            if (format.empty() || format == "d" || format == "D")
            {
                platform->text = format_date_time(*date, format == "D" ? "D" : "d");
            }
            else
            {
                platform->text = format_date_time(*date, format);
            }
        }
    } // namespace

    // Headless has no native view in the `native` slot, so destruction is trivial.
    date_picker_platform::~date_picker_platform() = default;

    std::unique_ptr<date_picker_platform> date_picker_handler::create_platform_view()
    {
        return std::make_unique<date_picker_platform>();
    }

    void date_picker_handler::on_connect_handler(date_picker_platform& platform)
    {
        // OnDoneClicked → SetVirtualViewDate: commit the dialog's current value to the virtual view
        // (whose own coercion clamps it into [MinimumDate, MaximumDate]).
        platform.on_done = [this] {
            auto* view = virtual_view();
            auto* typed = typed_platform_view();
            if (view == nullptr || typed == nullptr)
            {
                return;
            }
            view->set_date(typed->date);
        };
    }

    void date_picker_handler::on_disconnect_handler(date_picker_platform& platform)
    {
        platform.on_done = nullptr;
    }

    void date_picker_handler::map_format(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view); // UpdateFormat routes into UpdateDate (re-render the text)
    }

    void date_picker_handler::map_date(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view);
    }

    void date_picker_handler::map_minimum_date(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->minimum_date = view.minimum_date(); // UIDatePicker.MinimumDate
        }
    }

    void date_picker_handler::map_maximum_date(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum_date = view.maximum_date();
        }
    }

    void date_picker_handler::map_text_color(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void date_picker_handler::map_font(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void date_picker_handler::map_character_spacing(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void date_picker_handler::map_is_open(date_picker_handler& /*handler*/, i_date_picker& /*view*/)
    {
        // DatePickerHandler.MapIsOpen → become/resign first responder on the native field. Headless
        // has no native dialog (and no editing-begin/end callback to fire back), so this is a genuine
        // no-op — the control-level is_open()/Opened/Closed are the observable result.
    }

    maui::graphics::size date_picker_handler::get_desired_size(double width_constraint,
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

    void date_picker_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
