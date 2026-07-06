#pragma once
// maui::samples::varied_size_selector_page — ports
//   DataTemplateSelectorGalleries/VariedSizeDataTemplateSelectorGallery.xaml
//   (+ VariedSizeDataTemplateSelectorGallery.xaml.cs) of the C# CollectionView gallery.
//
// The C# page: a Grid (RowDefinitions="*,Auto") with a CollectionView on top
// (ItemSizingStrategy="MeasureAllItems", ItemTemplate = a DrinkTemplateSelector) over a control panel
// below — three Buttons (Insert / Add / Remove), an Index Entry ({Binding Index}), and a Picker
// ("Coffee"/"Milk"/"Latte", SelectedItem {Binding SelectedTemplate}). The selector returns one of three
// DataTemplates of DIFFERENT HEIGHTS per item TYPE:
//   - MilkTemplate   → a Wheat Border, HeightRequest 100, a Label {Binding Name};
//   - CoffeeTemplate → a SaddleBrown Border, HeightRequest 50, a Label {Binding Name};
//   - LatteTemplate  → a BurlyWood Border (auto height), a Label {Binding Name, '{0}'} plus a nested
//     BindableLayout over the Latte's Ingredients (each ingredient's Name).
// The xaml.cs seeds Items with Coffee0/Milk1/Coffee2/Coffee3/Milk4/Coffee5; the buttons Insert/Add/
// Remove a freshly-built drink of the picked template at the Index; Latte drinks carry two ingredients
// (a Milk + a Coffee). DrinkTemplateSelector.OnSelectTemplate branches on `item is Coffee/Milk/Latte`.
//
// This headless port owns its whole tree (the data_template_selector_page pattern) and mirrors the
// selector + the varied-height templates + the control panel code-first:
//   - drink is the reflection-free DrinkBase: a `kind` discriminator (coffee/milk/latte) + Name +
//     (for lattes) an ingredient Name list. The reflection-free port can't do C#'s `item is Coffee`
//     type test (all items share type_tag::of<drink>), so the selector branches on `kind` — the exact
//     same per-item decision, keyed on the discriminator instead of the runtime type (the
//     data_template_selector_page demo_item precedent);
//   - drink_selector is a data_template_selector subclass returning milk_template_ / coffee_template_ /
//     latte_template_ off each item's kind (the DrinkTemplateSelector logic);
//   - each template is a varied-HEIGHT cell: Milk stages HeightRequest 100, Coffee 50, Latte auto
//     (the visible "varied size" point), each a Label bound to drink.Name (Latte's label folds in its
//     ingredient names — see note);
//   - the control panel (buttons + Index entry + template picker) drives Insert/Add/Remove via the
//     same IsValid(index) guard + CreateDrink(SelectedTemplate) the xaml.cs uses, mutating the live
//     ObservableCollection<DrinkBase>.
//
// note: the C# templates are Borders wrapping a Label (+ Latte's nested BindableLayout of ingredient
//       Labels). The port's templated cells render a SINGLE root control (data_template::of<TControl>),
//       so each cell is the bound Label and the Border's Background COLOR is staged on the cell as the
//       label's background (the color-per-type signal is preserved); the varied HEIGHT is staged via the
//       cell's HeightRequest (the actual "varied size" the demo showcases). The Latte's nested
//       Ingredients BindableLayout has no single-root analog, so the Latte cell's Label folds the drink
//       Name plus its ingredient names into one bound string (documented reduction; the ingredient DATA
//       is preserved, just rendered as one label instead of a nested list). ItemSizingStrategy
//       MeasureAllItems is set on the view (the headless sim stores it; with varied per-cell heights it
//       is the meaningful strategy — every item measured individually).
//
// AT-REST board policy (EQUIVALENCE_FINDINGS "gap corpus" note): the shared twin cannot express the
// DrinkTemplateSelector (the loader has no reflection to activate the C# selector classes), so it
// degrades every cell to ONE uniform template — a Wheat, HeightRequest-100, Padding-8 cell with the
// bound Name label — and drops the picker's {Binding SelectedTemplate} preselect. The MAUI reference
// (the twin's render) is the board's ground truth, so the builder renders those SAME at-rest cells and
// leaves the picker unselected; the real 3-way selector machinery below (drink_selector +
// build_templates) stays in code as the future gap_<feature>.xaml scenario, deliberately UNWIRED at
// rest. The Insert/Add/Remove/selection interactivity is untouched.

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class varied_size_selector_page
    {
    public:
        // The reflection-free DrinkBase hierarchy collapsed to one struct with a `kind` discriminator
        // (the selector branches on this instead of C#'s `item is Coffee/Milk/Latte` runtime test —
        // header note). Name is the Label binding; ingredients carries the Latte's two ingredient Names.
        enum class drink_kind
        {
            coffee = 0,
            milk,
            latte,
        };

        struct drink
        {
            drink_kind kind = drink_kind::coffee;
            std::string name;
            std::vector<std::string> ingredients; // non-empty only for lattes
            friend bool operator==(const drink&, const drink&) = default;
        };

        // DrinkTemplateSelector.OnSelectTemplate: branch on the drink's kind (header note).
        class drink_selector : public maui::controls::data_template_selector
        {
        public:
            std::shared_ptr<maui::controls::data_template> coffee_template;
            std::shared_ptr<maui::controls::data_template> milk_template;
            std::shared_ptr<maui::controls::data_template> latte_template;

        protected:
            std::shared_ptr<maui::controls::data_template> on_select_template(
                const item_box& item, maui::core::bindable_object* /*container*/) override
            {
                if (item.value && item.type == maui::core::type_tag::of<drink>())
                {
                    const auto* row = static_cast<const drink*>(item.value.get());
                    switch (row->kind)
                    {
                        case drink_kind::coffee:
                            return coffee_template;
                        case drink_kind::milk:
                            return milk_template;
                        case drink_kind::latte:
                            return latte_template;
                    }
                }
                // C# OnSelectTemplate throws ArgumentOutOfRangeException for an unknown item; the port
                // falls back to the coffee template (the selector must return a non-null template).
                return coffee_template;
            }
        };

        varied_size_selector_page()
            : items_(std::make_shared<maui::core::observable_collection<drink>>(create_default_drinks()))
        {
            page_.set_title("Varied Size DataTemplateSelector");

            // ---- the Grid (RowDefinitions="*,Auto") ----
            grid_.add_row_definition(maui::core::grid_length::star());      // the collection view
            grid_.add_row_definition(maui::core::grid_length::automatic()); // the control panel

            // ---- the templates: the 3-way selector is BUILT (gap-corpus machinery, header note) but
            // the AT-REST wiring is the twin's single uniform Wheat cell ----
            build_templates();
            list_.set_item_template(build_uniform_twin_template());
            list_.set_item_sizing_strategy(maui::controls::item_sizing_strategy::measure_all_items);
            list_.set_items_source(items_);

            // ---- the control panel: Insert / Add / Remove buttons, the Index entry, the template picker ----
            insert_.set_text("Insert");
            add_.set_text("Add");
            remove_.set_text("Remove");
            insert_.clicked.connect([this] { on_insert(); });
            add_.clicked.connect([this] { on_add(); });
            remove_.clicked.connect([this] { on_remove(); });

            index_label_.set_text("Index");
            index_entry_.set_text(index_); // {Binding Index} initial "1"
            // {Binding Index}: keep the model field in sync as the user edits (the C# two-way Entry —
            // Entry.TextChanged fires (oldValue, newValue)).
            index_entry_.text_changed.connect(
                [this](const std::string& /*old_value*/, const std::string& new_value) { index_ = new_value; });

            template_picker_.set_title("Select a template");
            template_picker_.set_items_source(std::make_shared<maui::controls::picker::items_source_type>(
                std::vector<std::string>{"Coffee", "Milk", "Latte"}));
            // NO at-rest preselect: the twin drops {Binding SelectedTemplate} (header note), so the MAUI
            // reference shows the empty "Select a template" placeholder; selected_template_ still seeds
            // "Latte" internally (the C# _selectedTemplate initial) for the Insert/Add drink factory.
            // {Binding SelectedTemplate}: track the picked template name.
            template_picker_.selected_index_changed.connect([this] {
                if (const auto& picked = template_picker_.selected_item())
                {
                    selected_template_ = *picked;
                }
            });

            // The buttons row (a 3-column grid, like the XAML).
            button_row_.add_column_definition(maui::core::grid_length::star());
            button_row_.add_column_definition(maui::core::grid_length::star());
            button_row_.add_column_definition(maui::core::grid_length::star());
            button_row_.set_column(insert_, 0);
            button_row_.add(insert_);
            button_row_.set_column(add_, 1);
            button_row_.add(add_);
            button_row_.set_column(remove_, 2);
            button_row_.add(remove_);

            panel_.set_spacing(8);
            panel_.add(button_row_);
            panel_.add(index_label_);
            panel_.add(index_entry_);
            panel_.add(template_picker_);

            grid_.set_row(list_, 0);
            grid_.add(list_);
            grid_.set_row(panel_, 1);
            grid_.add(panel_);
            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- the xaml.cs button handlers (IsValid(index) guard + the live-collection mutation) ----

        // Insert_OnClicked: insert a freshly-built drink at the Index (no-op on an invalid index).
        void on_insert()
        {
            int index = 0;
            if (!is_valid(index))
            {
                return;
            }
            items_->insert(static_cast<std::size_t>(index), create_drink());
        }

        // Add_OnClicked: append a freshly-built drink (the index is only validated, not used — C#).
        void on_add()
        {
            int index = 0;
            if (!is_valid(index))
            {
                return;
            }
            items_->add(create_drink());
        }

        // Remove_OnClicked: remove the drink at the Index (no-op on an invalid index).
        void on_remove()
        {
            int index = 0;
            if (!is_valid(index))
            {
                return;
            }
            items_->remove_at(static_cast<std::size_t>(index));
        }

        // The owned controls + model, exposed for the hosting main's bottom-up attachment / tests.
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::button& insert_button()
        {
            return insert_;
        }
        [[nodiscard]] maui::controls::button& add_button()
        {
            return add_;
        }
        [[nodiscard]] maui::controls::button& remove_button()
        {
            return remove_;
        }
        [[nodiscard]] maui::controls::entry& index_entry()
        {
            return index_entry_;
        }
        [[nodiscard]] maui::controls::picker& template_picker()
        {
            return template_picker_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<drink>>& items() const
        {
            return items_;
        }

        // Test/host seam for the two-way model fields (the xaml.cs Index / SelectedTemplate properties).
        void set_index(std::string value)
        {
            index_ = std::move(value);
            index_entry_.set_text(index_);
        }
        void set_selected_template(std::string value)
        {
            selected_template_ = std::move(value);
            template_picker_.set_selected_item(selected_template_);
        }

    private:
        // CreateDefaultDrinks(): Coffee0, Milk1, Coffee2, Coffee3, Milk4, Coffee5 (the xaml.cs seed).
        [[nodiscard]] static std::vector<drink> create_default_drinks()
        {
            return {
                make_coffee("0"), make_milk("1"), make_coffee("2"), make_coffee("3"), make_milk("4"), make_coffee("5"),
            };
        }

        // CreateDrink(): build a drink of the picked template (the xaml.cs switch on SelectedTemplate);
        // a Latte carries a Milk + a Coffee ingredient (the xaml.cs Ingredients seed).
        [[nodiscard]] drink create_drink()
        {
            const std::string suffix = std::to_string(counter_++);
            if (selected_template_ == "Milk")
            {
                return make_milk(suffix);
            }
            if (selected_template_ == "Coffee")
            {
                return make_coffee(suffix);
            }
            // "Latte" (the xaml.cs default): two ingredients (a Milk + a Coffee).
            const std::string milk_suffix = std::to_string(counter_++);
            const std::string coffee_suffix = std::to_string(counter_++);
            drink latte = make_latte(suffix);
            latte.ingredients = {"Milk" + milk_suffix, "Coffee" + coffee_suffix};
            return latte;
        }

        // The three drink factories (C# `new Coffee(name)` → Name = "Coffee" + name, etc.).
        [[nodiscard]] static drink make_coffee(const std::string& name)
        {
            return drink{drink_kind::coffee, "Coffee" + name, {}};
        }
        [[nodiscard]] static drink make_milk(const std::string& name)
        {
            return drink{drink_kind::milk, "Milk" + name, {}};
        }
        [[nodiscard]] static drink make_latte(const std::string& name)
        {
            return drink{drink_kind::latte, "Latte" + name, {}};
        }

        // IsValid(out index): the xaml.cs guard — Index must parse to an int in [0, Items.Count].
        [[nodiscard]] bool is_valid(int& index)
        {
            index = -1;
            if (index_.empty())
            {
                return false;
            }
            try
            {
                std::size_t consumed = 0;
                const int parsed = std::stoi(index_, &consumed);
                if (consumed != index_.size())
                {
                    return false; // trailing non-numeric chars (C# int.TryParse fails)
                }
                index = parsed;
            }
            catch (const std::exception&)
            {
                return false; // not an int (C# int.TryParse fails)
            }
            return index >= 0 && static_cast<std::size_t>(index) <= items_->size();
        }

        // The three varied-height templates + the selector (header note: the Border becomes the cell's
        // background color; the varied HeightRequest is the "varied size" the demo is about).
        // The at-rest cell: the twin's single uniform DataTemplate — Wheat background, HeightRequest
        // 100, Padding 8, the bound Name label vertically centered (the Border root's look folded onto
        // the single-root label cell, the same reduction stage_background documents).
        [[nodiscard]] static std::shared_ptr<maui::controls::data_template> build_uniform_twin_template()
        {
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, drink>(maui::controls::label::text_property(),
                                                  [](const drink& d) { return d.name; });
            cell->set_value(maui::controls::height_request_property(), 100.0);
            cell->set_value(maui::controls::label::padding_property(), maui::core::thickness(8));
            stage_background(cell, maui::graphics::colors::wheat);
            return cell;
        }

        void build_templates()
        {
            // MilkTemplate: Wheat background, HeightRequest 100, Name label.
            auto milk = maui::controls::data_template::of<maui::controls::label>();
            milk->set_binding<std::string, drink>(maui::controls::label::text_property(),
                                                  [](const drink& d) { return d.name; });
            milk->set_value(maui::controls::height_request_property(), 100.0);
            stage_background(milk, maui::graphics::colors::wheat);

            // CoffeeTemplate: SaddleBrown background, HeightRequest 50, Name label.
            auto coffee = maui::controls::data_template::of<maui::controls::label>();
            coffee->set_binding<std::string, drink>(maui::controls::label::text_property(),
                                                    [](const drink& d) { return d.name; });
            coffee->set_value(maui::controls::height_request_property(), 50.0);
            stage_background(coffee, maui::graphics::colors::saddle_brown);

            // LatteTemplate: BurlyWood background, AUTO height, a label folding Name + ingredient names
            // (the nested Ingredients BindableLayout reduction — header note).
            auto latte = maui::controls::data_template::of<maui::controls::label>();
            latte->set_binding<std::string, drink>(maui::controls::label::text_property(), [](const drink& d) {
                std::string text = "Drink name is: " + d.name;
                if (!d.ingredients.empty())
                {
                    text += "     The ingredients are: ";
                    for (const std::string& ingredient : d.ingredients)
                    {
                        text += "    " + ingredient;
                    }
                }
                return text;
            });
            stage_background(latte, maui::graphics::colors::burly_wood);

            auto selector = std::make_shared<drink_selector>();
            selector->coffee_template = std::move(coffee);
            selector->milk_template = std::move(milk);
            selector->latte_template = std::move(latte);
            selector_ = std::move(selector);
        }

        // Stage a cell's Border background color as the label's own Background (the single-root reduction
        // — header note: the per-type color signal is preserved on the cell).
        static void stage_background(const std::shared_ptr<maui::controls::data_template>& cell,
                                     maui::graphics::color color)
        {
            cell->set_value(
                maui::controls::background_property(),
                std::static_pointer_cast<maui::graphics::paint>(std::make_shared<maui::graphics::solid_paint>(color)));
            // Center the bound Name text vertically within the cell's fixed height (Milk 100 / Coffee 50).
            // The oracle (VariedSizeSelectorPage.cs) sets `VerticalOptions = LayoutOptions.Center` on each
            // cell Label; here the cell IS that label (single-root reduction) and it also carries the cell's
            // background color, so it must FILL the row to paint the full-height color band. VerticalOptions
            // would shrink the label to text height and lose the fill, so the faithful equivalent is
            // VerticalTextAlignment=Center: full-height colored cell with the Name centered in it.
            cell->set_value(maui::controls::label::vertical_text_alignment_property(),
                            maui::core::text_alignment::center);
        }

        std::shared_ptr<maui::core::observable_collection<drink>> items_; // publisher before the list (§8)
        std::shared_ptr<maui::controls::data_template> selector_;         // the DrinkTemplateSelector

        // The xaml.cs model fields.
        std::string index_ = "1";                 // {Binding Index}
        std::string selected_template_ = "Latte"; // {Binding SelectedTemplate} (nameof(Latte))
        int counter_ = 6;                         // _counter (the next drink's name suffix)

        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::collection_view list_;
        maui::controls::vertical_stack_layout panel_;
        maui::controls::grid button_row_;
        maui::controls::button insert_;
        maui::controls::button add_;
        maui::controls::button remove_;
        maui::controls::label index_label_;
        maui::controls::entry index_entry_;
        maui::controls::picker template_picker_;
    };
} // namespace maui::samples
