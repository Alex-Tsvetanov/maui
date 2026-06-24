#pragma once
// maui::samples::scroll_mode_test_page — ports ScrollModeTestGallery.xaml (+ .xaml.cs) of the C#
//   CollectionView gallery
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.ScrollModeGalleries.ScrollModeTestGallery).
//
// The original page: a 6-row Grid. Rows 1–4 hold four buttons — "Scroll To Middle", "Add Item Above",
// "Add Item Below", "Add Item To End". Row 0 also gets an EnumSelector<ItemsUpdatingScrollMode> added in
// code-behind (a label + a picker that gets/sets _collectionView.ItemsUpdatingScrollMode), and row 5
// holds the CollectionView. The CollectionView is created in the ctor (default LinearItemsLayout.Vertical,
// ExampleTemplates.PhotoTemplate()), bound to a DemoFilteredItemSource(20).Items — an
// ObservableCollection<CollectionViewGalleryTestItem> of 20 captioned items. The button handlers:
//   - ScrollToMiddle  → CollectionView.ScrollTo(Items.Count / 2, position: Start, animate: false);
//   - AddItemAbove    → Items.Insert((Count / 2) - 1, new "Inserted item");
//   - AddItemBelow    → Items.Insert((Count / 2) + 2, new "Inserted item");
//   - AddItemToEnd    → Items.Add(new "Added item").
//
// This gallery exists to test scroll behaviors: how ScrollTo lands a target item (Start position, no
// animation) and how inserting/appending items interacts with the active ItemsUpdatingScrollMode (whether
// the viewport stays put, holds its offset, or follows the new item). It is the interactive counterpart to
// ItemsUpdatingScrollModeGallery — same mode selector, but with explicit ScrollTo + positional inserts.
//
// This headless port owns its whole tree and reproduces all of that code-first (the scroll_to_group_page +
// items_page patterns):
//   - test_item { caption }: the reflection-free CollectionViewGalleryTestItem (we keep the Caption the
//     template surfaces; Date/Image/Index and the More/Less ICommands the page never uses are dropped — as
//     filter_collection_page's photo_item does);
//   - items_ : the live observable_collection<test_item> bound to the collection_view
//     (DemoFilteredItemSource.Items), seeded with 20 items cycling the demo image-name captions;
//   - the PhotoTemplate stand-in is a Label bound to test_item.caption (note: the C# PhotoTemplate is an
//     image + caption Grid; the headless sim measures the displayable caption text);
//   - set_items_updating_scroll_mode wires the mode; the EnumSelector is a caption Label + a Picker over the
//     three ItemsUpdatingScrollMode values (KeepItemsInView / KeepScrollOffset / KeepLastItemInView) whose
//     SelectedIndexChanged gets/sets ItemsUpdatingScrollMode — the faithful EnumSelector<T> shape (a single
//     value shown, matching MAUI, rather than three side-by-side buttons that overflow the row);
//   - scroll_to_middle()/add_item_above()/add_item_below()/add_item_to_end() reproduce the four C# handlers
//     verbatim (ScrollTo with Start/no-animate; the two positional Inserts; the Add). The ScrollTo request
//     is observed through the collection_view's scroll_to_requested event into a readout (the
//     scroll_to_group_page precedent — the headless sim raises the request without a native viewport), and
//     the readout also reports the active mode + live count.
//
// note: the C# ctor takes optional itemsLayout / dataTemplate / createCollectionView factories (the gallery
//   instantiates it several ways — vertical, horizontal, grid); this port fixes the default vertical linear
//   layout + the photo/caption template, the shape the page's own default path uses.

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/scroll_to_request_event_args.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::samples
{
    class scroll_mode_test_page
    {
    public:
        // The reflection-free CollectionViewGalleryTestItem: the Caption the template surfaces.
        struct test_item
        {
            std::string caption;
            friend bool operator==(const test_item&, const test_item&) = default;
        };

        scroll_mode_test_page() : items_(std::make_shared<maui::core::observable_collection<test_item>>())
        {
            page_.set_title("ScrollModeTest Gallery");
            root_.set_spacing(8);

            // ---- the EnumSelector: caption + a Picker over the three ItemsUpdatingScrollMode values
            // (the C# code-behind adds an EnumSelector<ItemsUpdatingScrollMode> = a Label + a Picker that
            // gets/sets _collectionView.ItemsUpdatingScrollMode). The Picker's SelectedIndexChanged drives
            // set_mode; index 0 (KeepItemsInView) is the initial selection. ----
            mode_caption_.set_text("ItemsUpdatingScrollMode:");
            mode_items_->add("KeepItemsInView");
            mode_items_->add("KeepScrollOffset");
            mode_items_->add("KeepLastItemInView");
            mode_picker_.set_items_source(mode_items_);
            mode_picker_.set_selected_index(0); // KeepItemsInView (the default mode_)
            mode_picker_.selected_index_changed.connect([this] { on_mode_picked(); });
            mode_bar_.set_spacing(8);
            mode_bar_.add(mode_caption_);
            mode_bar_.add(mode_picker_);

            // ---- rows 1–4: the four action buttons (verbatim C# AutomationId text) ----
            scroll_to_middle_.set_text("Scroll To Middle");
            add_above_.set_text("Add Item Above");
            add_below_.set_text("Add Item Below");
            add_to_end_.set_text("Add Item To End");
            scroll_to_middle_.clicked.connect([this] { scroll_to_middle(); });
            add_above_.clicked.connect([this] { add_item_above(); });
            add_below_.clicked.connect([this] { add_item_below(); });
            add_to_end_.clicked.connect([this] { add_item_to_end(); });

            // ---- row 5: the CollectionView (default vertical linear layout + the photo/caption template —
            // see note) ----
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, test_item>(maui::controls::label::text_property(),
                                                      [](const test_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_items_layout(std::make_shared<maui::controls::linear_items_layout>(
                maui::controls::items_layout_orientation::vertical)); // LinearItemsLayout.Vertical (default)
            list_.set_items_updating_scroll_mode(mode_);
            list_.set_items_source(items_);

            // Observe ScrollTo through the request event (no native viewport headless — scroll_to_group
            // precedent).
            list_.scroll_to_requested.connect(
                [this](const maui::controls::scroll_to_request_event_args& args) { on_scroll_requested(args); });

            seed_items(); // DemoFilteredItemSource(20): 20 captioned items
            update_readout();

            root_.add(mode_bar_);
            root_.add(scroll_to_middle_);
            root_.add(add_above_);
            root_.add(add_below_);
            root_.add(add_to_end_);
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
        [[nodiscard]] maui::controls::button& scroll_to_middle_button()
        {
            return scroll_to_middle_;
        }
        [[nodiscard]] maui::controls::button& add_above_button()
        {
            return add_above_;
        }
        [[nodiscard]] maui::controls::button& add_below_button()
        {
            return add_below_;
        }
        [[nodiscard]] maui::controls::button& add_to_end_button()
        {
            return add_to_end_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<test_item>>& items() const
        {
            return items_;
        }
        [[nodiscard]] maui::controls::items_updating_scroll_mode mode() const
        {
            return mode_;
        }

        // The EnumSelector setter: _collectionView.ItemsUpdatingScrollMode = mode.
        void set_mode(maui::controls::items_updating_scroll_mode value)
        {
            mode_ = value;
            list_.set_items_updating_scroll_mode(mode_);
            update_readout();
        }

        // ScrollToMiddle_Clicked: ScrollTo(Items.Count / 2, position: Start, animate: false).
        void scroll_to_middle()
        {
            const int target = static_cast<int>(items_->size()) / 2;
            list_.scroll_to(target, /*group_index*/ -1, maui::controls::scroll_to_position::start,
                            /*is_animated*/ false);
        }

        // AddItemAbove_Clicked: Items.Insert((Count / 2) - 1, new "Inserted item").
        void add_item_above()
        {
            const int index = (static_cast<int>(items_->size()) / 2) - 1;
            insert_at(index, "Inserted item");
        }

        // AddItemBelow_Clicked: Items.Insert((Count / 2) + 2, new "Inserted item").
        void add_item_below()
        {
            const int index = (static_cast<int>(items_->size()) / 2) + 2;
            insert_at(index, "Inserted item");
        }

        // AddItemToEnd_Clicked: Items.Add(new "Added item").
        void add_item_to_end()
        {
            items_->add(test_item{"Added item"});
            update_readout();
        }

    private:
        // DemoFilteredItemSource(20): 20 captioned items cycling the demo image names ("<image>, <n>").
        void seed_items()
        {
            static const char* const images[] = {"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                 "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            constexpr int count = 20;
            constexpr int image_count = static_cast<int>(std::size(images));
            for (int n = 0; n < count; ++n)
            {
                items_->add(test_item{std::string{images[n % image_count]} + ", " + std::to_string(n)});
            }
        }

        // Items.Insert(index, ...): the C# index is always in range for the live count here, but clamp to
        // [0, size] so a degenerate small list can't throw (observable_collection::insert requires
        // index <= size).
        void insert_at(int index, const std::string& caption)
        {
            const int size = static_cast<int>(items_->size());
            const int clamped = index < 0 ? 0 : (index > size ? size : index);
            items_->insert(static_cast<std::size_t>(clamped), test_item{caption});
            update_readout();
        }

        // The ScrollTo request payload (raised by scroll_to(...) — no native viewport headless): report
        // the target landing.
        void on_scroll_requested(const maui::controls::scroll_to_request_event_args& args)
        {
            readout_.set_text("Scrolled to index " + std::to_string(args.index) + " (Start, no animation)  ·  " +
                              "Mode: " + mode_name(mode_) + "  ·  Items: " + std::to_string(items_->size()));
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

        // The Picker's SelectedIndexChanged (the EnumSelector setter): map the chosen row to the mode.
        void on_mode_picked()
        {
            switch (mode_picker_.selected_index())
            {
                case 1:
                    set_mode(maui::controls::items_updating_scroll_mode::keep_scroll_offset);
                    break;
                case 2:
                    set_mode(maui::controls::items_updating_scroll_mode::keep_last_item_in_view);
                    break;
                default:
                    set_mode(maui::controls::items_updating_scroll_mode::keep_items_in_view);
                    break;
            }
        }

        void update_readout()
        {
            readout_.set_text(std::string{"Mode: "} + mode_name(mode_) +
                              "  ·  Items: " + std::to_string(items_->size()));
        }

        std::shared_ptr<maui::core::observable_collection<test_item>> items_; // publisher first (§8)
        maui::controls::items_updating_scroll_mode mode_ =
            maui::controls::items_updating_scroll_mode::keep_items_in_view;
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::horizontal_stack_layout mode_bar_;
        maui::controls::label mode_caption_;
        // The EnumSelector's Picker over the three ItemsUpdatingScrollMode values + its backing list
        // (the picker tracks a maui::controls::observable_collection<std::string>, distinct from the
        // maui::core::observable_collection the collection_view items use).
        std::shared_ptr<maui::controls::observable_collection<std::string>> mode_items_ =
            std::make_shared<maui::controls::observable_collection<std::string>>();
        maui::controls::picker mode_picker_;
        maui::controls::button scroll_to_middle_;
        maui::controls::button add_above_;
        maui::controls::button add_below_;
        maui::controls::button add_to_end_;
        maui::controls::label readout_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
