#pragma once
// maui::samples::empty_view_selector_page — ports
// EmptyViewGalleries/EmptyViewWithDataTemplateSelector.xaml (+ .xaml.cs, incl. its
// SearchTermDataTemplateSelector + EmptyViewWithDataTemplateSelectorViewModel).
//
// The C# gallery page is a Grid (RowDefinitions="Auto, Auto, *"): three instruction Labels (Row 0), a
// SearchBar (Row 1, x:Name searchBar) whose SearchCommand="{Binding FilterCommand}" and
// SearchCommandParameter="{Binding Source={x:Reference searchBar}, Path=Text}", and a CollectionView
// (Row 2) with ItemsSource="{Binding Monkeys}", an ItemTemplate (Name + Location Labels), and
// EmptyView="{Binding Source={x:Reference searchBar}, Path=Text}" — i.e. the EmptyView object IS the
// current SearchBar text. The Resources declare TWO empty-view DataTemplates:
//   - AdvancedTemplate → a StackLayout of two Labels: "No results matched your filter." (Bold 18) +
//     "Try a broader filter?" (Italic 12);
//   - BasicTemplate    → one Label "No items to display." (Bold 18).
// The xaml.cs ctor wires a SearchTermDataTemplateSelector as the CollectionView.EmptyViewTemplate:
//   DefaultTemplate = AdvancedTemplate, OtherTemplate = BasicTemplate;
//   OnSelectTemplate(item, container): query = (string)item;
//       return query.Equals("xamarin", OrdinalIgnoreCase) ? OtherTemplate : DefaultTemplate;
// The view model's FilterCommand(text) keeps the Monkeys whose Name contains the term (the page seeds one
// Monkey, "Baboon"). So: filtering to a term that matches no monkey empties the source and the
// CollectionView renders the EmptyViewTemplate the SELECTOR picks off the term — BasicTemplate when the
// term is exactly "xamarin", otherwise AdvancedTemplate. That is the demonstrated feature: an
// EmptyViewTemplate chosen by a DataTemplateSelector keyed on the empty-view item (the search string).
//
// Port mapping (mirrors data_template_selector_page's EmptyViewTemplate selector half + the filter
// plumbing of the sibling EmptyView pages):
//   - term_selector is a data_template_selector subclass choosing other_template_ (Basic) vs
//     default_template_ (Advanced) off whether the current empty-view term equals "xamarin"
//     case-insensitively (the C# SearchTermDataTemplateSelector.OnSelectTemplate logic);
//   - set_empty_view_template(term_selector) wires it; the collection_view_handler's resolve_template
//     calls select_template per empty-view realization, so the headless virtualization simulator
//     exercises the selection directly;
//   - the search_bar drives FilterCommand through its `search_command` (the C# SearchCommand /
//     FilterCommand stand-in): filter_items(term) keeps the matching monkeys, then set_empty_view = the
//     term (so the EmptyViewTemplate selector sees the search string — the C# EmptyView={searchBar.Text}
//     binding, re-published on each search);
//   - ItemsSource = the Monkeys observable_collection (seeded with one "Baboon", the C# view model).
//
// note: the C# ItemTemplate is a Grid with Name + Location Labels; the port template here is a single
//       Label reading "<Name> — <Location>" (no headless multi-cell layout binding needed for the
//       demonstrated intent — same single-Label reduction as the sibling CollectionView pages).
// note: the C# AdvancedTemplate is a StackLayout of two Labels; a data_template::of<label> creates ONE
//       content root, so the port AdvancedTemplate is a single Label folding both lines
//       ("No results matched your filter. Try a broader filter?") and the BasicTemplate is the single
//       Label "No items to display." Both are type-activated of<label>() so they render natively; the
//       Bold/Italic/FontSize styling of the original is dropped to the default font (documented; the
//       distinguishing TEXT — which template the selector picked — is what the demo proves).
//
// The page OWNS its whole element tree (the items_page pattern); the generic mount (app_host.hpp) attaches
// every owned view's handler and hosts the grid + page.

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::samples
{
    class empty_view_selector_page
    {
    public:
        // One row of the demo source — the Monkey the ItemTemplate binds + the filter matches against
        // (the C# Monkey { Name, Location, Details }, reduced to the surfaced Name + Location).
        struct monkey
        {
            std::string name;
            std::string location;
            friend bool operator==(const monkey&, const monkey&) = default;
        };

        // SearchTermDataTemplateSelector: OtherTemplate (Basic) if the term equals "xamarin"
        // case-insensitively, else DefaultTemplate (Advanced). The EmptyView item is the search string
        // (the C# CollectionView.EmptyView = {searchBar.Text}); the selector casts it to string.
        class term_selector : public maui::controls::data_template_selector
        {
        public:
            std::shared_ptr<maui::controls::data_template> default_template; // AdvancedTemplate
            std::shared_ptr<maui::controls::data_template> other_template;   // BasicTemplate

        protected:
            std::shared_ptr<maui::controls::data_template> on_select_template(
                const item_box& item, maui::core::bindable_object* /*container*/) override
            {
                if (item.value && item.type == maui::core::type_tag::of<std::string>())
                {
                    const auto& term = *static_cast<const std::string*>(item.value.get());
                    if (equals_ignore_case(term, "xamarin"))
                    {
                        return other_template;
                    }
                }
                return default_template;
            }

        private:
            // C# string.Equals(query, "xamarin", OrdinalIgnoreCase).
            [[nodiscard]] static bool equals_ignore_case(const std::string& left, const std::string& right)
            {
                if (left.size() != right.size())
                {
                    return false;
                }
                for (std::size_t i = 0; i < left.size(); ++i)
                {
                    if (std::tolower(static_cast<unsigned char>(left[i])) !=
                        std::tolower(static_cast<unsigned char>(right[i])))
                    {
                        return false;
                    }
                }
                return true;
            }
        };

        empty_view_selector_page() : items_(std::make_shared<maui::core::observable_collection<monkey>>(source_items()))
        {
            page_.set_title("EmptyView (template selector)");

            // Grid: "Auto, Auto, *" — the instruction stack, the SearchBar, then the CollectionView.
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());

            // ---- Row 0: the three instruction Labels, folded into one (the C# instruction StackLayout) ----
            instructions_.set_text("1. Filter the items below by search term.  "
                                   "2. Filtering 'Xamarin' (no matched results) shows 'No items to display.'  "
                                   "3. Filtering to no data (except Xamarin) shows 'No results matched your filter.' "
                                   "and 'Try a broader filter?'");

            // ---- Row 1: the SearchBar; SearchCommand = FilterCommand(text) (the C# binding) ----
            search_.set_placeholder("Filter");
            search_.search_command = [this] {
                const std::string term = std::string(search_.text());
                filter_items(term);
                // EmptyView = the search text (the C# EmptyView={searchBar.Text}); the selector reads it.
                list_.set_empty_view(maui::controls::boxed_item::of(term));
            };

            // ---- Row 2: the CollectionView ----
            // ItemTemplate: a single Label "<Name> — <Location>" (see note).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, monkey>(maui::controls::label::text_property(), [](const monkey& value) {
                return value.name + " — " + value.location;
            });
            list_.set_item_template(cell);

            build_empty_view_selector();
            list_.set_empty_view_template(empty_view_selector_);
            list_.set_items_source(items_);

            grid_.set_row(instructions_, 0);
            grid_.add(instructions_);
            grid_.set_row(search_, 1);
            grid_.add(search_);
            grid_.set_row(list_, 2);
            grid_.add(list_);
            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::search_bar& search()
        {
            return search_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<monkey>>& items() const
        {
            return items_;
        }

        // The C# FilterCommand body: keep the monkeys whose Name contains the term (case-insensitive),
        // Clear, then re-Add the matches. An empty filter keeps every row (the Contains short-circuit on
        // ""). The C# FilterItems rebuilds the live collection from a filtered snapshot of the backing
        // set; the port does the same against the unfiltered source_ so the EmptyView/selector path is
        // reached when nothing matches.
        void filter_items(const std::string& filter)
        {
            items_->clear();
            for (const monkey& item : source_)
            {
                if (name_contains(item.name, filter))
                {
                    items_->add(item);
                }
            }
        }

        // The explicit empty/fill toggle (clear -> the EmptyViewTemplate appears; fill restores).
        void clear_items()
        {
            items_->clear();
        }
        void fill_items()
        {
            filter_items("");
        }

    private:
        // SearchTermDataTemplateSelector's two empty-view templates (Advanced default / Basic other) —
        // each a single Label folding the original StackLayout/Label text (see note).
        void build_empty_view_selector()
        {
            auto advanced = maui::controls::data_template::of<maui::controls::label>();
            advanced->set_value(maui::controls::label::text_property(),
                                std::string{"No results matched your filter. Try a broader filter?"});

            auto basic = maui::controls::data_template::of<maui::controls::label>();
            basic->set_value(maui::controls::label::text_property(), std::string{"No items to display."});

            auto selector = std::make_shared<term_selector>();
            selector->default_template = std::move(advanced);
            selector->other_template = std::move(basic);
            empty_view_selector_ = std::move(selector);
        }

        // The C# EmptyViewWithDataTemplateSelectorViewModel seed: one Monkey, "Baboon".
        [[nodiscard]] static std::vector<monkey> source_items()
        {
            return {monkey{"Baboon", "Africa & Asia"}};
        }

        // Case-insensitive substring (C# Name.Contains(filter, OrdinalIgnoreCase)); "" matches all.
        [[nodiscard]] static bool name_contains(const std::string& name, const std::string& filter)
        {
            if (filter.empty())
            {
                return true;
            }
            const auto lower = [](std::string text) {
                std::transform(text.begin(), text.end(), text.begin(),
                               [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                return text;
            };
            return lower(name).find(lower(filter)) != std::string::npos;
        }

        std::vector<monkey> source_ = source_items();                        // the unfiltered backing list
        std::shared_ptr<maui::core::observable_collection<monkey>> items_;   // publisher before the list (§8)
        std::shared_ptr<maui::controls::data_template> empty_view_selector_; // SearchTermDataTemplateSelector
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::label instructions_; // the Row-0 instruction text
        maui::controls::search_bar search_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
