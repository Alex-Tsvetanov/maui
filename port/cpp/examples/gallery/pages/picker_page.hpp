#pragma once
// maui::samples::picker_page — ports PickerPage.xaml (+ PickerPage.xaml.cs)
//
// A self-contained, code-first demo page for the Picker control (the C# gallery-page convention,
// mirroring the value_controls_page / pickers_page pattern). The page OWNS its whole element tree;
// `page()` hands back the content_page; the generic mount (app_host.hpp) attaches every owned view's
// handler and hosts the tree.
//
// The C# page is a scroll of headline-labelled Picker variants exercising a wide property surface.
// This port keeps the cross-platform-API subset and wires the demonstrated INTERACTIONS so every
// selection drives a visible readout:
//   - "Basic" picker: a 20-item ItemsSource (MorePickerItems), Title placeholder.
//   - "SelectedIndex=1": an ItemsSource picker pre-selected to index 1.
//   - "SelectedIndexChanged": a picker whose selection drives the readout (the C# DisplayAlert
//     collapses to a label readout — headless-safe).
//   - "TextColor=Blue" / "TitleColor=Blue" / "FontAttributes=Italic + BackgroundColor=Yellow":
//     styling flavors (the porting subset the picker control exposes).
//   - "Dynamic add items": the Items face (add/clear/replace) driven by buttons (the C# Loaded +
//     Clear/Add/Replace handlers).
//   - "Items markup": an Items-list picker (the C# <Picker.Items> x:String children) recolored green.
//
// Faithful best-effort deviations (// note:):
//   - HorizontalOptions/VerticalOptions, HorizontalTextAlignment/VerticalTextAlignment-as-layout, the
//     IsOpen Open/Close buttons, the Background gradient buttons and the Changing-BindingContext block
//     are markup-/binding-era surface; the cross-platform layer here exposes the picker's own
//     properties + Items face, so those map to the closest API or are noted, never invented.
//   - C#'s ItemDisplayBinding ("{Binding Name}") is string-only in the port (the display value IS the
//     string item), so the BindingContext block collapses to a plain Items picker.

#include <memory>
#include <string>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class picker_page
    {
    public:
        picker_page()
        {
            page_.set_title("Picker");
            stack_.set_spacing(6);
            stack_.set_padding(maui::core::thickness(12));

            // The shared item set (C#'s PickerItems = {Item 1, Item 2, Item 3}) plus the 20-item
            // MorePickerItems for the Basic picker.
            basic_label_.set_text("Basic");
            basic_picker_.set_title("Select an item");
            for (int i = 1; i <= 20; ++i)
            {
                basic_picker_.items().add("Item " + std::to_string(i));
            }

            // "SelectedIndex=1" — mirror the XAML DOCUMENT ORDER: the SelectedIndex attribute is applied
            // BEFORE the <Picker.Items> element children, so Picker.CoerceSelectedIndex clamps it to -1
            // against the still-empty Items (and the later item adds re-clamp -1 to -1). The at-rest
            // render is therefore the "Select an item" placeholder, exactly like the MAUI reference —
            // setting the index AFTER the items had it stick at 1 ("Item 2"), a genuine order divergence.
            preselect_label_.set_text("SelectedIndex=1");
            preselect_picker_.set_title("Select an item");
            preselect_picker_.set_selected_index(1); // coerced to -1 (Items still empty), like the XAML
            add_three_items(preselect_picker_);

            // "SelectedIndexChanged" — the selection drives the readout (the C# DisplayAlert collapses
            // to a visible label here, headless-safe).
            changed_label_.set_text("SelectedIndexChanged");
            readout_.set_text("Selected: (none)");
            changed_picker_.set_title("Select an item");
            changed_picker_.set_selected_index(1); // XAML attribute order: coerced to -1 (see above)
            add_three_items(changed_picker_);
            changed_picker_.selected_index_changed.connect([this] { update_readout(); });

            // "TextColor=Blue".
            text_color_label_.set_text("TextColor=Blue");
            text_color_picker_.set_title("Select an item");
            add_three_items(text_color_picker_);
            text_color_picker_.set_text_color(maui::graphics::colors::blue);

            // "TitleColor=Blue".
            title_color_label_.set_text("TitleColor=Blue");
            title_color_picker_.set_title("Select an item");
            add_three_items(title_color_picker_);
            title_color_picker_.set_title_color(maui::graphics::colors::blue);

            // "FontAttributes=Italic + BackgroundColor=Yellow".
            styled_label_.set_text("FontAttributes=Italic + BackgroundColor=Yellow");
            styled_picker_.set_title("Select an item");
            add_three_items(styled_picker_);
            styled_picker_.set_font(maui::core::font::default_font().with_slant(maui::core::font_slant::italic));
            styled_picker_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::yellow));

            // "Dynamic add items" — the Items face driven by buttons (C#'s Loaded seeds 3 items, then
            // Clear/Add/Replace). The picker starts with the 3 items the C# Loaded handler adds.
            dynamic_label_.set_text("Dynamic add items to the Picker");
            dynamic_picker_.set_title("The picker should have 3 items.");
            add_three_items(dynamic_picker_);
            clear_items_button_.set_text("Clear Items");
            clear_items_button_.clicked.connect([this] { dynamic_picker_.items().clear(); });
            add_items_button_.set_text("Add Items");
            add_items_button_.clicked.connect([this] {
                dynamic_picker_.items().add("New Item 1");
                dynamic_picker_.items().add("New Item 2");
                dynamic_picker_.items().add("New Item 3");
            });
            replace_items_button_.set_text("Replace Items");
            replace_items_button_.clicked.connect([this] {
                dynamic_picker_.items().clear();
                dynamic_picker_.items().add("Item One");
                dynamic_picker_.items().add("Item Two");
                dynamic_picker_.items().add("Item Three");
            });

            // "Items markup" — the C# <Picker.Items> x:String children, recolored green/white.
            markup_label_.set_text("Items (markup) + BackgroundColor=Green");
            markup_picker_.set_selected_index(0); // XAML attribute order: coerced to -1 (see above)
            markup_picker_.items().add("Item 1");
            markup_picker_.items().add("Item 2");
            markup_picker_.items().add("Item 3");
            markup_picker_.set_text_color(maui::graphics::colors::white);
            markup_picker_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));

            // note: the C# IsOpen Open/Close buttons (Picker.IsOpen + Opened/Closed) and the gradient
            // Background buttons need a hosted native dialog / a brush-rebuild; out of scope for this
            // headless-safe code-first cut, so they are omitted rather than stubbed.

            stack_.add(basic_label_);
            stack_.add(basic_picker_);
            stack_.add(preselect_label_);
            stack_.add(preselect_picker_);
            stack_.add(changed_label_);
            stack_.add(changed_picker_);
            stack_.add(readout_);
            stack_.add(text_color_label_);
            stack_.add(text_color_picker_);
            stack_.add(title_color_label_);
            stack_.add(title_color_picker_);
            stack_.add(styled_label_);
            stack_.add(styled_picker_);
            stack_.add(dynamic_label_);
            stack_.add(dynamic_picker_);
            stack_.add(clear_items_button_);
            stack_.add(add_items_button_);
            stack_.add(replace_items_button_);
            stack_.add(markup_label_);
            stack_.add(markup_picker_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::picker& basic_picker()
        {
            return basic_picker_;
        }
        [[nodiscard]] maui::controls::picker& changed_picker()
        {
            return changed_picker_;
        }
        [[nodiscard]] maui::controls::picker& dynamic_picker()
        {
            return dynamic_picker_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        static void add_three_items(maui::controls::picker& target)
        {
            target.items().add("Item 1");
            target.items().add("Item 2");
            target.items().add("Item 3");
        }

        void update_readout()
        {
            const int index = changed_picker_.selected_index();
            const std::string selected = index >= 0 ? changed_picker_.get_item(index) : std::string("(none)");
            readout_.set_text("Selected: " + selected);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label basic_label_;
        maui::controls::picker basic_picker_;
        maui::controls::label preselect_label_;
        maui::controls::picker preselect_picker_;
        maui::controls::label changed_label_;
        maui::controls::picker changed_picker_;
        maui::controls::label readout_;
        maui::controls::label text_color_label_;
        maui::controls::picker text_color_picker_;
        maui::controls::label title_color_label_;
        maui::controls::picker title_color_picker_;
        maui::controls::label styled_label_;
        maui::controls::picker styled_picker_;
        maui::controls::label dynamic_label_;
        maui::controls::picker dynamic_picker_;
        maui::controls::button clear_items_button_;
        maui::controls::button add_items_button_;
        maui::controls::button replace_items_button_;
        maui::controls::label markup_label_;
        maui::controls::picker markup_picker_;
    };
} // namespace maui::samples
