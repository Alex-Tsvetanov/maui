// picker_handler — headless platform recipe. A testable stand-in for the native picker field: every
// mapped property is mirrored into picker_platform, the items/selection maps run the C#
// PickerExtensions.UpdatePicker algorithm against the mirrors (display text + items + the virtual
// write-back), and on_done is the Done-accessory commit (FinishSelectItem) tests invoke directly to
// simulate a native row pick. The iOS backend (src/platform/ios/picker_handler.mm) is the real-native
// twin; AppKit translates to NSPopUpButton.

#include "maui/core/picker_handler.hpp"

#include <cstddef>
#include <memory>
#include <string>

#include "maui/core/i_picker.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    namespace
    {
        // PickerExtensions.UpdatePicker(platformPicker, picker, newSelectedIndex): refresh the display
        // text + the item list, then write the selection back to the virtual view (skipped while empty).
        void update_picker(picker_handler& handler, i_picker& view, int selected_index)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            platform->text = selected_index != -1 ? view.get_item(selected_index) : std::string{};

            const int count = view.get_count();
            platform->items.clear();
            platform->items.reserve(static_cast<std::size_t>(count));
            for (int at = 0; at < count; ++at)
            {
                platform->items.push_back(view.get_item(at));
            }

            if (count == 0)
            {
                return;
            }
            platform->selected_index = selected_index;
            view.set_selected_index(selected_index); // picker.SelectedIndex = selectedIndex (FromHandler)
        }
    } // namespace

    // Headless has no native view in the `native` slot, so destruction is trivial.
    picker_platform::~picker_platform() = default;

    std::unique_ptr<picker_platform> picker_handler::create_platform_view()
    {
        return std::make_unique<picker_platform>();
    }

    void picker_handler::on_connect_handler(picker_platform& platform)
    {
        // FinishSelectItem: commit the native wheel's pending row — an unset (-1) row with items
        // present selects row 0 — then push the text + the virtual selection.
        platform.on_done = [this](int row) {
            auto* view = virtual_view();
            if (view == nullptr)
            {
                return;
            }
            if (row == -1 && view->get_count() > 0)
            {
                row = 0;
            }
            update_picker(*this, *view, row);
        };
    }

    void picker_handler::on_disconnect_handler(picker_platform& platform)
    {
        platform.on_done = nullptr;
    }

    void picker_handler::map_items(picker_handler& handler, i_picker& view)
    {
        update_picker(handler, view, view.selected_index()); // Reload -> UpdatePicker(picker)
    }

    void picker_handler::map_selected_index(picker_handler& handler, i_picker& view)
    {
        update_picker(handler, view, view.selected_index()); // UpdateSelectedIndex
    }

    void picker_handler::map_title(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->title = std::string(view.title()); // UpdatePickerTitle (the placeholder)
        }
    }

    void picker_handler::map_title_color(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->title_color = view.title_color();
        }
    }

    void picker_handler::map_text_color(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void picker_handler::map_font(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void picker_handler::map_character_spacing(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void picker_handler::map_horizontal_text_alignment(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->horizontal_alignment = view.horizontal_text_alignment();
        }
    }

    void picker_handler::map_vertical_text_alignment(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
        }
    }

    maui::graphics::size picker_handler::get_desired_size(double width_constraint, double /*height_constraint*/) const
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

    void picker_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
