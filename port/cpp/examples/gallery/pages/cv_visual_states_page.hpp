#pragma once
// maui::samples::cv_visual_states_page — ports CollectionViewGalleries/SelectionGalleries/
// VisualStatesGallery.xaml (+ .xaml.cs) of the C# CollectionView gallery.
//
// The original page (VisualStatesGallery): a VerticalStackLayout holding two labeled CollectionViews:
//   - "Single Selection": SelectionMode="Single",   ItemsSource={Binding SingleSelectionItems} — three
//     LineItems ("Item 1".."Item 3");
//   - "Multi Selection":  SelectionMode="Multiple", ItemsSource={Binding MultiSelectionItems} — four
//     LineItems ("Item 1".."Item 4").
// Both use ItemSizingStrategy="MeasureFirstItem" and EmptyView="No items defined". The point of the
// gallery is the cell's DataTemplate: a root Grid (BackgroundColor=White) that carries a
// VisualStateManager.VisualStateGroups CommonStates group —
//   - Normal   → (no setter, the White base);
//   - Selected → Setter BackgroundColor="Yellow".
// Inside the root Grid is a nested Grid > Label bound to {Binding ItemName} (NoWrap, FontSize=Large).
// So selecting a row turns its background Yellow via the system-driven CommonStates VSM on the cell.
//
// The port mirrors this with the selectable_items_view selection surface + the per-cell VSM:
//   - line_item: the reflection-free LineItem — just the ItemName the cell label binds;
//   - each CollectionView gets a data_template::of<label> cell bound to line_item.ItemName, NoWrap +
//     a Large-ish font (FontSize="Large"), AND a CommonStates VSM group (Normal/Selected) staged onto
//     the created cell content via add_cell_visual_states() so each realized cell carries the group —
//     the closest faithful code-first analog of the XAML's VisualStateManager.VisualStateGroups on the
//     cell's root view;
//   - selection_mode is Single / Multiple respectively; item_sizing_strategy is measure_first_item;
//     empty_view is the boxed "No items defined" string.
//
// note: the XAML's Selected setter recolors BackgroundColor=Yellow. In the port BackgroundColor is a
//       PAINT (view::background), not a `bindable_property<color>`, so a Setter on a color property uses
//       TextColor as the faithful color stand-in (the same convention the sibling visual_states_page.hpp
//       documents): Selected => TextColor=Yellow, Normal => TextColor=White-base. The state-transition
//       behavior (Normal<->Selected swap on selection) is identical.
// note: the per-cell CommonStates VSM is SYSTEM-DRIVEN (Selected is a system state the collection's
//       selection drives). The headless backend's virtualization sim realizes cells but does not run a
//       cell's change_visual_state on selection, so the per-cell recolor has no headless VISUAL — this
//       is the documented "per-cell VSM may have no headless visual" case. The group is still staged on
//       every realized cell (and would drive a real backend), and selection ITSELF is fully observable
//       through the selectable_items_view surface (selected_item() / selected_items()), exercised by the
//       select_single / select_multiple drivers below.
// note: the XAML nests the bound Label inside two Grids (a root Grid carrying the VSM + an inner Grid).
//       The nesting has no headless-visible geometric effect, so — exactly like the grouping siblings —
//       the cell is the bound Label directly, and the VSM group is staged on that Label (the cell root).

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::samples
{
    class cv_visual_states_page
    {
    public:
        // The reflection-free LineItem: just the ItemName the cell label binds (ToString => ItemName).
        struct line_item
        {
            std::string item_name;
            friend bool operator==(const line_item&, const line_item&) = default;
        };

        cv_visual_states_page()
            : single_items_(std::make_shared<maui::core::observable_collection<line_item>>()),
              multi_items_(std::make_shared<maui::core::observable_collection<line_item>>())
        {
            page_.set_title("VisualStates");
            stack_.set_spacing(4);

            // ItemsSource = SingleSelectionItems (3) / MultiSelectionItems (4).
            single_items_->add(line_item{"Item 1"});
            single_items_->add(line_item{"Item 2"});
            single_items_->add(line_item{"Item 3"});

            multi_items_->add(line_item{"Item 1"});
            multi_items_->add(line_item{"Item 2"});
            multi_items_->add(line_item{"Item 3"});
            multi_items_->add(line_item{"Item 4"});

            // ---- "Single Selection" CollectionView ----
            single_headline_.set_text("Single Selection");
            configure_list(single_list_, maui::controls::selection_mode::single);
            single_list_.set_items_source(single_items_);

            // ---- "Multi Selection" CollectionView ----
            multi_headline_.set_text("Multi Selection");
            configure_list(multi_list_, maui::controls::selection_mode::multiple);
            multi_list_.set_items_source(multi_items_);

            stack_.add(single_headline_);
            stack_.add(single_list_);
            stack_.add(multi_headline_);
            stack_.add(multi_list_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- headless selection drivers (selection is observable; the per-cell recolor is not — note:)
        // Select one item in the Single CollectionView by index (drives selected_item, the system Selected
        // state a real backend would apply to that cell).
        void select_single(std::size_t index)
        {
            if (index < single_items_->size())
            {
                single_list_.set_selected_item(maui::controls::boxed_item::of(single_items_->at(index)));
            }
        }
        // Select several items in the Multi CollectionView by index (drives selected_items()).
        void select_multiple(const std::vector<std::size_t>& indices)
        {
            std::vector<maui::controls::boxed_item> chosen;
            chosen.reserve(indices.size());
            for (const std::size_t index : indices)
            {
                if (index < multi_items_->size())
                {
                    chosen.push_back(maui::controls::boxed_item::of(multi_items_->at(index)));
                }
            }
            multi_list_.update_selected_items(chosen);
        }

        // ---- accessors (used by the hosting main + any test tree) ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::collection_view& single_list()
        {
            return single_list_;
        }
        [[nodiscard]] maui::controls::collection_view& multi_list()
        {
            return multi_list_;
        }

    private:
        // Both lists share the same cell shape (a bound NoWrap Large label at the White base look), the
        // same MeasureFirstItem sizing, and the same "No items defined" empty view — only the
        // selection_mode differs.
        //
        // note (per-cell CommonStates VSM — best-effort): the XAML puts a VisualStateManager
        //       .VisualStateGroups CommonStates group (Normal / Selected=>BackgroundColor=Yellow) on each
        //       cell's ROOT view. A data_template clones a fresh content per cell via its loader, and the
        //       port exposes no per-instance hook to stage a visual_state_manager onto each clone — the
        //       template's value/binding seam carries only bindable_property setters, and a VSM manager is
        //       not one. So that per-cell group cannot be attached from a code-first template here. This
        //       is the documented limit: the SELECTION the group reacts to is fully modeled + observable
        //       (selected_item / selected_items, driven by select_single / select_multiple), but the
        //       per-cell Selected=>Yellow recolor has no headless analog. The base White look (the cell's
        //       Normal/default) IS staged as TextColor=White (BackgroundColor is a paint, not a
        //       bindable_property<color>, so TextColor is the faithful color stand-in — header note); a
        //       real backend that ran each cell's change_visual_state on selection (with the group staged)
        //       would show the Normal<->Selected swap.
        void configure_list(maui::controls::collection_view& list, maui::controls::selection_mode mode)
        {
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, line_item>(maui::controls::label::text_property(),
                                                      [](const line_item& item) { return item.item_name; });
            // FontSize="Large" + LineBreakMode="NoWrap".
            cell->set_value(maui::controls::label::font_property(), maui::core::font::system_font_of_size(20));
            cell->set_value(maui::controls::label::line_break_mode_property(), maui::core::line_break_mode::no_wrap);
            // C# Normal state = the cell's Grid BackgroundColor=White with the Label's DEFAULT text color
            // (black in light / white in dark), so the item text is VISIBLE on the white cell. The port
            // leaves the cell label's text color at the system default (UILabel.labelColor — adaptive +
            // visible) rather than staging an explicit white (which had rendered white-on-white = the items
            // looked missing). The Selected=>Yellow recolor is the per-cell CommonStates VSM (system-driven);
            // staging that group per cell is the documented struct-cell-template limit (see header note), so
            // only the visible Normal/base look is reproduced here.
            list.set_item_template(cell);

            list.set_selection_mode(mode);
            list.set_item_sizing_strategy(maui::controls::item_sizing_strategy::measure_first_item);
            list.set_empty_view(maui::controls::boxed_item::of(std::string{"No items defined"}));
        }

        std::shared_ptr<maui::core::observable_collection<line_item>> single_items_; // publishers before lists (§8)
        std::shared_ptr<maui::core::observable_collection<line_item>> multi_items_;
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label single_headline_;
        maui::controls::collection_view single_list_;
        maui::controls::label multi_headline_;
        maui::controls::collection_view multi_list_;
    };
} // namespace maui::samples
