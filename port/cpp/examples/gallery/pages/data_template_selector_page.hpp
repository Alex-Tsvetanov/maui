#pragma once
// maui::samples::data_template_selector_page — ports DataTemplateSelectorGallery.xaml
// (+ DataTemplateSelectorGallery.xaml.cs, including its WeekendSelector + SearchTermSelector classes).
//
// The C# gallery page is a Grid (a SearchBar over a CollectionView) that demonstrates TWO
// DataTemplateSelectors at once:
//   - ItemTemplate = WeekendSelector: per item, it inspects the item's Date.DayOfWeek and returns the
//     WeekendTemplate for Saturday/Sunday, else the DefaultTemplate (the C# OnSelectTemplate);
//   - EmptyViewTemplate = SearchTermSelector: when the source filters to empty, it inspects the search
//     term and returns the SymbolsTemplate if the term contains any non-letter, else the DefaultTemplate
//     (EmptyTemplate). The xaml.cs sets EmptyView = the search text on each search, so the EmptyView item
//     the selector sees is that string.
// The source is a DemoFilteredItemSource(200, ItemMatches) filtered by day-of-week name. This port
// mirrors both selectors code-first:
//   - day_selector is a data_template_selector subclass choosing weekend_template_ vs default_template_
//     off each item's day_of_week (the WeekendSelector logic);
//   - term_selector is a data_template_selector subclass choosing symbols_template_ vs empty_template_
//     off whether the current empty-view term has a non-letter (the SearchTermSelector logic);
//   - set_item_template(day_selector) and set_empty_view_template(term_selector) wire them — the
//     collection_view_handler's resolve_template calls select_template per realized cell / per empty-view
//     realization (collection_view_handler.cpp), so the headless virtualization simulator exercises the
//     item selection directly.
//
// The page OWNS its whole element tree (the items_page pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window.
//
// note: the C# DefaultTemplate/WeekendTemplate are Grids with an Image + a Label; the port templates are
//       the Labels only (no headless image asset pipeline — same reduction as the sibling CollectionView
//       pages). The DefaultTemplate Label binds the item's day name (C# StringFormat '{0:dddd}'); the
//       WeekendTemplate Label is the fixed "It's the weekend! Woot!" string. The EmptyTemplate/Symbols
//       Labels bind the term itself (C# Binding(".") with a StringFormat), reproduced as a self-path
//       string binding. The C# day-of-week FILTER (ItemMatches over Date.DayOfWeek) and EmptyView=term
//       assignment are wired through the search_bar's search_command exactly as the xaml.cs does.

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
    class data_template_selector_page
    {
    public:
        // One row of the demo source. `day_of_week` is 0=Sunday..6=Saturday (C# DateTime.DayOfWeek);
        // `day_name` is the rendered weekday label (the DefaultTemplate's '{0:dddd}' binding).
        struct demo_item
        {
            int day_of_week = 0;
            std::string day_name;
        };

        // WeekendSelector: WeekendTemplate for Sat/Sun, else DefaultTemplate (the C# OnSelectTemplate).
        class day_selector : public maui::controls::data_template_selector
        {
        public:
            std::shared_ptr<maui::controls::data_template> default_template;
            std::shared_ptr<maui::controls::data_template> weekend_template;

        protected:
            std::shared_ptr<maui::controls::data_template> on_select_template(
                const item_box& item, maui::core::bindable_object* /*container*/) override
            {
                // All rows share type_tag::of<demo_item>; read the boxed value to branch on its day.
                if (item.value && item.type == maui::core::type_tag::of<demo_item>())
                {
                    const auto* row = static_cast<const demo_item*>(item.value.get());
                    if (row->day_of_week == 0 /*Sunday*/ || row->day_of_week == 6 /*Saturday*/)
                    {
                        return weekend_template;
                    }
                }
                return default_template;
            }
        };

        // SearchTermSelector: SymbolsTemplate if the term has a non-letter, else DefaultTemplate (the
        // EmptyTemplate). The EmptyView item is the search string (the C# CollectionView.EmptyView = term).
        class term_selector : public maui::controls::data_template_selector
        {
        public:
            std::shared_ptr<maui::controls::data_template> default_template;
            std::shared_ptr<maui::controls::data_template> symbols_template;

        protected:
            std::shared_ptr<maui::controls::data_template> on_select_template(
                const item_box& item, maui::core::bindable_object* /*container*/) override
            {
                if (item.value && item.type == maui::core::type_tag::of<std::string>())
                {
                    const auto& term = *static_cast<const std::string*>(item.value.get());
                    const bool has_symbol =
                        std::any_of(term.begin(), term.end(), [](unsigned char ch) { return std::isalpha(ch) == 0; });
                    if (has_symbol)
                    {
                        return symbols_template;
                    }
                }
                return default_template;
            }
        };

        data_template_selector_page()
            : items_(std::make_shared<maui::core::observable_collection<demo_item>>(source_items()))
        {
            page_.set_title("DataTemplateSelector");

            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());

            search_.set_placeholder("Day of Week Filter");
            // SearchBar.SearchCommand: filter by day-of-week name, then set EmptyView = the term (so the
            // EmptyViewTemplate selector sees the search string) — the C# xaml.cs ctor wiring, verbatim.
            search_.search_command = [this] {
                const std::string term = std::string(search_.text());
                filter_items(term);
                list_.set_empty_view(maui::controls::boxed_item::of(term));
            };

            build_item_selector();
            build_empty_view_selector();

            list_.set_item_template(item_selector_);
            list_.set_empty_view_template(empty_view_selector_);
            list_.set_items_source(items_);

            grid_.set_row(search_, 0);
            grid_.add(search_);
            grid_.set_row(list_, 1);
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
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<demo_item>>& items() const
        {
            return items_;
        }

        // The C# DemoFilteredItemSource(200, ItemMatches): keep rows whose day-of-week NAME contains the
        // filter (case-insensitive); an empty filter keeps every row. The C# FilterItems reconciles the
        // live source row-by-row against the backing list; here the rows REPEAT every 7 days (200 rows,
        // seven distinct day pairs), so a per-row identity reconcile is ambiguous — the port rebuilds the
        // live source from the backing list instead (same observable result: matching rows present,
        // non-matching absent, so the EmptyView/selector path is reached when nothing matches).
        void filter_items(const std::string& filter)
        {
            items_->clear();
            for (const demo_item& item : source_)
            {
                if (name_contains(item.day_name, filter))
                {
                    items_->add(item);
                }
            }
        }

    private:
        // WeekendSelector's two templates (DefaultTemplate day-name Label / WeekendTemplate fixed Label).
        void build_item_selector()
        {
            auto default_template = maui::controls::data_template::of<maui::controls::label>();
            default_template->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                                  [](const demo_item& item) { return item.day_name; });

            auto weekend_template = maui::controls::data_template::of<maui::controls::label>();
            weekend_template->set_value(maui::controls::label::text_property(), std::string{"It's the weekend! Woot!"});

            auto selector = std::make_shared<day_selector>();
            selector->default_template = std::move(default_template);
            selector->weekend_template = std::move(weekend_template);
            item_selector_ = std::move(selector);
        }

        // SearchTermSelector's two templates (EmptyTemplate / SymbolsTemplate) — both bind the term itself
        // (the C# Binding(".") with a StringFormat), reproduced as a self-path string binding.
        void build_empty_view_selector()
        {
            auto empty_template = maui::controls::data_template::of<maui::controls::label>();
            empty_template->set_binding<std::string, std::string>(
                maui::controls::label::text_property(),
                [](const std::string& term) { return "(" + term + ") does not match any day of the week."; });

            auto symbols_template = maui::controls::data_template::of<maui::controls::label>();
            symbols_template->set_binding<std::string, std::string>(
                maui::controls::label::text_property(), [](const std::string& term) {
                    return "(" + term + ") _definitely_ does not match any day of the week.";
                });

            auto selector = std::make_shared<term_selector>();
            selector->default_template = std::move(empty_template);
            selector->symbols_template = std::move(symbols_template);
            empty_view_selector_ = std::move(selector);
        }

        // DemoFilteredItemSource(200): 200 rows, day-of-week stepping one day per index (the C#
        // DateTime.Now.AddDays(n) sequence, seeded deterministically here so cells are predictable).
        [[nodiscard]] static std::vector<demo_item> source_items()
        {
            static const std::vector<std::string> day_names{"Sunday",   "Monday", "Tuesday", "Wednesday",
                                                            "Thursday", "Friday", "Saturday"};
            std::vector<demo_item> rows;
            for (int n = 0; n < 200; ++n)
            {
                const int dow = n % 7;
                rows.push_back(demo_item{dow, day_names[static_cast<std::size_t>(dow)]});
            }
            return rows;
        }

        // Case-insensitive substring (C# DayOfWeek.ToString().Contains(filter, OrdinalIgnoreCase)); ""
        // matches all (the C# IsNullOrEmpty short-circuit returns true).
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

        std::vector<demo_item> source_ = source_items();                      // the unfiltered backing list
        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // publisher before the list (§8)
        std::shared_ptr<maui::controls::data_template> item_selector_;        // WeekendSelector
        std::shared_ptr<maui::controls::data_template> empty_view_selector_;  // SearchTermSelector
        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::search_bar search_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
