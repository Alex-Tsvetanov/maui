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
//   - each CollectionView gets a data_template::of<line_item_cell> cell — line_item_cell is a Grid that
//     OWNS a bound NoWrap Large-font (FontSize="Large") Label child, matching the twin's Grid-rooted
//     cell structure (see line_item_cell's own doc comment for why the child must be owned, not just
//     referenced);
//   - selection_mode is Single / Multiple respectively; item_sizing_strategy is measure_first_item;
//     empty_view is the boxed "No items defined" string.
//
// note: the cell's base (Normal) look is the XAML Grid's EXPLICIT BackgroundColor=White — staged per
//       cell via background_property (a bindable_property<shared_ptr<paint>>), so the white item bands
//       survive the dark theme exactly like the MAUI reference (explicit colors are never
//       theme-overridden). The Selected setter's BackgroundColor=Yellow recolor rides the same
//       property when a backend drives the per-cell CommonStates VSM.
// note: the per-cell CommonStates VSM is SYSTEM-DRIVEN (Selected is a system state the collection's
//       selection drives). The headless backend's virtualization sim realizes cells but does not run a
//       cell's change_visual_state on selection, so the per-cell recolor has no headless VISUAL — this
//       is the documented "per-cell VSM may have no headless visual" case. The group is still staged on
//       every realized cell (and would drive a real backend), and selection ITSELF is fully observable
//       through the selectable_items_view surface (selected_item() / selected_items()), exercised by the
//       select_single / select_multiple drivers below.
// note: the XAML nests the bound Label inside two Grids (a root Grid carrying the VSM + an inner Grid).
//       The inner Grid has no headless- or native-visible geometric effect and is collapsed away, but the
//       OUTER Grid does: MAUI/the xaml twin measure the MeasureFirstItem row height off the actual
//       Grid-wrapped cell, which is measurably taller (natively) than a bare Label. PORT FIX (2026-07-06,
//       the cpp<->xaml consistency check): the cell root is now line_item_cell (a Grid owning the bound
//       Label), not the bound Label directly — see line_item_cell below.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/view.hpp" // background_property (the staged White cell background)
#include "maui/core/font.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/solid_paint.hpp"

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

        // The cell root: a Grid that OWNS its bound Label child (layout::add() only references children,
        // it does not own them — PROFILE §8 — so a data_template::of<TControl>() cell that needs to add a
        // freshly-created child must own that child as a member, exactly like this composite). Mirrors the
        // twin's <Grid BackgroundColor="White"><Grid><Label .../></Grid></Grid> (the outer Grid only; the
        // inner Grid has no headless- OR native-visible geometric effect and is collapsed away).
        class line_item_cell final : public maui::controls::grid
        {
        public:
            line_item_cell()
            {
                // A Grid with NO explicit RowDefinitions/ColumnDefinitions gets ONE implicit row/column —
                // but the port's grid_layout_manager (see make_rows/make_columns in
                // src/layouts/grid_layout_manager.cpp) treats that implicit cell as `*` (star), not
                // `Auto`. A star row/column has no natural size of its own: under MeasureFirstItem's
                // infinite/self-sizing measure pass it degenerates to a ZERO-height cell (a star needs a
                // finite constraint to compute its proportional share), which measured this whole
                // CollectionView to nothing. An explicit Auto row/column makes the Grid self-size to its
                // (single) child's natural size instead, matching a plain MAUI `<Grid>` with no
                // RowDefinitions (which defaults each axis to one Auto-equivalent star cell that DOES
                // shrink-to-fit when there's exactly one child and no siblings competing for space —
                // reproduced here explicitly since the port's implicit-star fallback does not shrink-to-fit
                // under an infinite constraint).
                add_row_definition(maui::core::grid_length::automatic());
                add_column_definition(maui::core::grid_length::automatic());
                // FontSize="Large" + LineBreakMode="NoWrap". NamedSize is PER-PLATFORM in MAUI (a separate
                // FontNamedSizeService per backend), so "Large" is NOT one number: it is 32 on Windows
                // (Platform/Windows/Extensions/FontExtensions.cs:40) and 22 on Apple
                // (Compatibility/iOS/FontNamedSizeService.cs). xaml_converters.cpp's convert_font_size now
                // resolves it per platform, so the shared-XAML twin gets the right value automatically --
                // this code-first page has to mirror that by hand because maui::core::font exposes only
                // system_font_of_size(double) with no named-size overload.
                // The previous literal 22.0 here carried a comment claiming it "matches MAUI's actual
                // measured cell/row height"; that was true of the Apple boards it was measured on and wrong
                // on Windows, where it left the code-first column 10pt short of both MAUI and the twin.
                // ponytail: a named-size overload on maui::core::font (or exposing convert_font_size to the
                // code-first layer) would make this a one-liner and remove the duplication -- worth doing if
                // a second page ever needs a named size; today cv_visual_states is the only one on the board.
#ifdef MAUI_PLATFORM_WINDOWS
                label_.set_font(maui::core::font::system_font_of_size(32.0));
#else
                label_.set_font(maui::core::font::system_font_of_size(22.0));
#endif
                label_.set_line_break_mode(maui::core::line_break_mode::no_wrap);
                add(label_);
            }

        protected:
            // Push {Binding .} (the context itself, mirroring the twin's inline x:Array of strings via
            // ItemName) → Label.Text when the cell's BindingContext (the line_item) is set by the realize
            // path — the same on_binding_context_changed hook as header_footer_template_page's photo_cell.
            void on_binding_context_changed() override
            {
                maui::controls::grid::on_binding_context_changed(); // propagate to children first
                if (const auto item = binding_context<line_item>())
                {
                    label_.set_text(item->item_name);
                }
            }

        private:
            maui::controls::label label_;
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

        // PRE-MOUNT hook (gallery_host.hpp gallery_pre_mount): register line_item_cell's handler BEFORE
        // mount_window / the collection_view realize walk. line_item_cell is a brand-new user type (like
        // header_footer_template_page's photo_cell), so its handler isn't self-registered; the
        // collection_view realize path resolves a template's handler via THIS app's per-app
        // handler_registry (of<TCell>() → create_handler by the cell's type_tag) — without this the native
        // cell realize silently no-ops (content_type() has no registered handler) and the lists render
        // blank. line_item_cell is a grid subclass, so it shares grid's layout_handler.
        void register_handlers(maui::hosting::maui_app& app)
        {
            maui::core::register_handler<line_item_cell, maui::core::layout_handler>(app.handlers());
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
        // Both lists share the same cell shape (a Grid-rooted line_item_cell at the White base look), the
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
        //       Normal/default Grid BackgroundColor=White) IS staged: background_property is a
        //       bindable_property<shared_ptr<paint>>, so the template's value seam carries the explicit
        //       white paint per cell (see configure_list); a real backend that ran each cell's
        //       change_visual_state on selection (with the group staged) would show the Normal<->Selected
        //       swap.
        void configure_list(maui::controls::collection_view& list, maui::controls::selection_mode mode)
        {
            // The cell root is a Grid (line_item_cell, matching the twin's <Grid BackgroundColor="White">
            // <Grid><Label/></Grid></Grid> — the outer Grid only; the inner Grid has no headless- OR
            // native-visible geometric effect and is collapsed away). PORT FIX (2026-07-06): this used to
            // be a bare Label root. That collapse IS geometrically invisible in the headless test harness,
            // but NOT on a real native backend (Mac Catalyst) — a bare Label measures ~3pt shorter per row
            // than MAUI's actual Grid-wrapped cell (measured natively: MAUI's own capture showed a
            // 120px-tall 3-item Single Selection CollectionView; the bare-Label builder measured only
            // 111px, a real 3pt/row native-Grid-vs-Label MeasureFirstItem delta), which is what the
            // cpp<->xaml consistency check caught (xaml's real Grid-rooted hydration already measured
            // 120px, matching MAUI). Wrapping the cell root in a Grid, with the bound Label as its owned
            // child, reproduces MAUI's actual measured row height.
            auto cell = maui::controls::data_template::of<line_item_cell>();
            // C# Normal state = the cell's Grid BackgroundColor=White (an EXPLICIT color the theme must
            // never override): stage it on the Grid root as a white background paint. The MAUI
            // reference retains the white bands in DARK mode too (the dark-theme adaptive label color
            // turns white, so the item text goes invisible on them — MAUI's own render, ground truth per
            // the standing doctrine); the port reproduces exactly that by leaving the text color at the system
            // default. The Selected=>Yellow recolor is the per-cell CommonStates VSM (system-driven);
            // staging that group per cell is the documented struct-cell-template limit (see header note),
            // so only the Normal/base look is reproduced here.
            cell->set_value(maui::controls::background_property(),
                            std::static_pointer_cast<maui::graphics::paint>(
                                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::white)));
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
