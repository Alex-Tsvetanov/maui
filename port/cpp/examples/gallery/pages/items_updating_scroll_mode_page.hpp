#pragma once
// maui::samples::items_updating_scroll_mode_page — ports ItemsUpdatingScrollModeGallery.xaml (+
//   .xaml.cs) of the C# CollectionView gallery
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ItemsUpdatingScrollModeGallery).
//
// The original page: a Grid (Auto / *). Row 0 is a horizontal StackLayout with an
// "UpdatingScrollMode:" Label + an EnumPicker<ItemsUpdatingScrollMode> (SelectedIndex 0 →
// KeepItemsInView). Its SelectedIndexChanged sets CollectionView.ItemsUpdatingScrollMode. Row 1 is the
// CollectionView bound to ItemsUpdatingScrollModeViewModel.Items — an ObservableCollection<{Text1,
// Text2}> filled by LoadItemsAsync (50 items added one-per-second on OnAppearing). Each cell is a 2-row
// Grid: a bold Text1 label over an italic Text2 label.
//
// The whole point of this gallery is ItemsUpdatingScrollMode: it governs where the viewport lands when
// items are ADDED to the live source — KeepItemsInView (default: the currently-visible items stay put),
// KeepScrollOffset (the absolute offset is preserved), or KeepLastItemInView (chat-style: the newest
// item is scrolled into view). The C# demo surfaces the effect by streaming 50 inserts while you toggle
// the mode; the headless port surfaces the same wiring deterministically.
//
// This headless port owns its whole tree and reproduces all of that code-first (the items_page +
// scroll_to_group_page patterns):
//   - scroll_item { text1, text2 }: the reflection-free ItemsUpdatingScrollModeItem;
//   - items_ : the live observable_collection<scroll_item> bound to the collection_view (the VM's
//     ObservableCollection<Items>), seeded with the same "Title N" / "Subtitle N" shape;
//   - the cell template is a Label bound to "Text1 — Text2" (note: the C# 2-row Grid of a bold + an
//     italic label is cosmetic two-line chrome; the headless sim measures the displayable text, so the
//     two bound strings are joined into one bound Label — both fields are still surfaced);
//   - set_items_updating_scroll_mode wires the mode. The EnumPicker is replaced by three explicit mode
//     buttons (KeepItemsInView / KeepScrollOffset / KeepLastItemInView) since there is no headless-safe
//     EnumPicker control on the surface (note:); each button sets the mode + refreshes the readout —
//     same SelectedIndexChanged → ItemsUpdatingScrollMode assignment, just driven discretely;
//   - an "Add Item" button appends one new Title/Subtitle to the live source (the per-tick body of
//     LoadItemsAsync, surfaced as a tap so the mode's effect on a live insert is observable headless,
//     without the OnAppearing async loop / Task.Delay), and the readout reports the current mode + the
//     live count.
//
// note: the C# LoadItemsAsync streams 50 items with a 1s Task.Delay between each on OnAppearing; the
//   headless port seeds an initial batch synchronously and exposes add_item()/the Add button for the
//   incremental inserts (the deterministic, clock-free equivalent — the mode-under-insert behavior this
//   gallery exists to show is preserved).

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::samples
{
    class items_updating_scroll_mode_page
    {
    public:
        // The reflection-free ItemsUpdatingScrollModeItem: the two strings the cell binds.
        struct scroll_item
        {
            std::string text1;
            std::string text2;
            friend bool operator==(const scroll_item&, const scroll_item&) = default;
        };

        items_updating_scroll_mode_page() : items_(std::make_shared<maui::core::observable_collection<scroll_item>>())
        {
            page_.set_title("ItemsUpdatingScrollMode Gallery");
            root_.set_spacing(12);

            // ---- row 0: the "UpdatingScrollMode:" Label + the three mode buttons (the EnumPicker
            // stand-in — note:) ----
            mode_caption_.set_text("UpdatingScrollMode:");
            keep_items_button_.set_text("KeepItemsInView");
            keep_offset_button_.set_text("KeepScrollOffset");
            keep_last_button_.set_text("KeepLastItemInView");
            keep_items_button_.clicked.connect(
                [this] { set_mode(maui::controls::items_updating_scroll_mode::keep_items_in_view); });
            keep_offset_button_.clicked.connect(
                [this] { set_mode(maui::controls::items_updating_scroll_mode::keep_scroll_offset); });
            keep_last_button_.clicked.connect(
                [this] { set_mode(maui::controls::items_updating_scroll_mode::keep_last_item_in_view); });
            mode_bar_.set_spacing(8);
            mode_bar_.add(mode_caption_);
            mode_bar_.add(keep_items_button_);
            mode_bar_.add(keep_offset_button_);
            mode_bar_.add(keep_last_button_);

            // ---- the Add Item button (the per-tick body of LoadItemsAsync, on tap) + the readout ----
            add_button_.set_text("Add Item");
            add_button_.clicked.connect([this] { add_item(); });

            // ---- row 1: the CollectionView (the two-line cell, see note) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, scroll_item>(
                maui::controls::label::text_property(),
                [](const scroll_item& item) { return item.text1 + " — " + item.text2; });
            list_.set_item_template(cell);
            list_.set_items_updating_scroll_mode(mode_); // EnumPicker SelectedIndex="0" → KeepItemsInView
            list_.set_items_source(items_);

            seed_items(); // a starting batch (the synchronous stand-in for LoadItemsAsync — note)
            update_readout();

            root_.add(mode_bar_);
            root_.add(add_button_);
            root_.add(readout_);
            root_.add(list_);
            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- owned controls exposed for the hosting main / tests ----
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::button& keep_items_button()
        {
            return keep_items_button_;
        }
        [[nodiscard]] maui::controls::button& keep_offset_button()
        {
            return keep_offset_button_;
        }
        [[nodiscard]] maui::controls::button& keep_last_button()
        {
            return keep_last_button_;
        }
        [[nodiscard]] maui::controls::button& add_button()
        {
            return add_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<scroll_item>>& items() const
        {
            return items_;
        }
        [[nodiscard]] maui::controls::items_updating_scroll_mode mode() const
        {
            return mode_;
        }

        // OnItemsUpdatingScrollModeChanged: CollectionView.ItemsUpdatingScrollMode = mode.
        void set_mode(maui::controls::items_updating_scroll_mode value)
        {
            mode_ = value;
            list_.set_items_updating_scroll_mode(mode_);
            update_readout();
        }

        // The per-tick body of LoadItemsAsync, surfaced as a tap: append one Title/Subtitle to the live
        // source. The active ItemsUpdatingScrollMode governs where the viewport lands for this insert.
        void add_item()
        {
            const int n = static_cast<int>(items_->size()) + 1;
            items_->add(scroll_item{"Title " + std::to_string(n), "Subtitle " + std::to_string(n)});
            update_readout();
        }

    private:
        // LoadItemsAsync seeds 50 "Title N" / "Subtitle N" items; the port seeds a deterministic starting
        // batch synchronously (the Add button supplies further inserts — see header note).
        void seed_items()
        {
            for (int i = 0; i < 50; ++i)
            {
                items_->add(scroll_item{"Title " + std::to_string(i + 1), "Subtitle " + std::to_string(i + 1)});
            }
        }

        [[nodiscard]] static const char* mode_name(maui::controls::items_updating_scroll_mode value)
        {
            switch (value)
            {
                case maui::controls::items_updating_scroll_mode::keep_items_in_view:
                    return "KeepItemsInView";
                case maui::controls::items_updating_scroll_mode::keep_scroll_offset:
                    return "KeepScrollOffset";
                case maui::controls::items_updating_scroll_mode::keep_last_item_in_view:
                    return "KeepLastItemInView";
            }
            return "KeepItemsInView";
        }

        void update_readout()
        {
            readout_.set_text(std::string{"Mode: "} + mode_name(mode_) +
                              "  ·  Items: " + std::to_string(items_->size()));
        }

        std::shared_ptr<maui::core::observable_collection<scroll_item>> items_; // publisher first (§8)
        maui::controls::items_updating_scroll_mode mode_ =
            maui::controls::items_updating_scroll_mode::keep_items_in_view; // EnumPicker SelectedIndex="0"
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::horizontal_stack_layout mode_bar_;
        maui::controls::label mode_caption_;
        maui::controls::button keep_items_button_;
        maui::controls::button keep_offset_button_;
        maui::controls::button keep_last_button_;
        maui::controls::button add_button_;
        maui::controls::label readout_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
