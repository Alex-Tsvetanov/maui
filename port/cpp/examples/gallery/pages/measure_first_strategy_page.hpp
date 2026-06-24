#pragma once
// maui::samples::measure_first_strategy_page — ports MeasureFirstStrategy.xaml (+ .xaml.cs) of the C#
//   CollectionView gallery
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.GroupingGalleries.MeasureFirstStrategy).
//
// The original page: a StackLayout with an explanatory Label ("Using ItemSizingStrategy.MeasureFirstItem.
// On scrolling, some items may disappear; this is a current known issue") over one grouped CollectionView
// with ItemSizingStrategy="MeasureFirstItem" and IsGrouped="True". Its ItemsSource = new SuperTeams() — a
// List<Team> where Team : List<Member> { string Name }. Three DataTemplates:
//   - ItemTemplate        → a StackLayout > Label bound to {Binding Name}                (each Member.Name);
//   - GroupHeaderTemplate → a LightGreen bold FontSize-16 Label bound to {Binding Name}  (each Team.Name);
//   - GroupFooterTemplate → an Orange Label bound to {Binding Count, StringFormat='Total members: {0:D}'}.
//
// This gallery exists to exercise ItemSizingStrategy.MeasureFirstItem: instead of measuring every item
// (MeasureAllItems), the layer measures the FIRST item and reuses that size for all of them — a perf
// strategy for uniform cells (the "known issue" note is the C# native quirk it was filed to repro). The
// varied-length member names + the grouped layout are the content it measures.
//
// This headless port owns its whole tree and reproduces all of that code-first — it is the
// basic_grouping_page shape (the canonical grouped-source pattern: item_collection<grouping_ptr>, each
// grouping carrying a KEY object the group header/footer bind against), specialized for the SuperTeams
// roster and pivoted onto the sizing strategy:
//   - member { name }: the reflection-free Member;
//   - team_key { name, count }: the reflection-free Team KEY half — Name (group header) + Count (group
//     footer; a build-time snapshot, since the reflection-free port can't bind C#'s List<T>.Count off the
//     Team object — the documented {Binding Count} stand-in, as in basic_grouping_page). The "list half"
//     of C#'s Team is the grouping's nested member collection;
//   - build_teams() reproduces SuperTeams() (the same six Marvel rosters);
//   - is_grouped + the three templates mirror the XAML (item Name, LightGreen group header, Orange
//     "Total members: N" group footer);
//   - set_item_sizing_strategy(measure_first_item) is the page's headline, verbatim from XAML.
//
// To make the strategy itself observable headless (the C# page is fixed at MeasureFirstItem), a toggle
// button flips MeasureFirstItem ↔ MeasureAllItems over the same varied content, and a readout reports the
// active strategy — the displayable signal the virtualization sim's measure pass keys on. The default is
// MeasureFirstItem (the XAML value).
//
// note: the C# ItemTemplate wraps the Name Label in a StackLayout (and the footer Label likewise); that
//   wrapper has no headless-visible effect, so each template binds the Label directly (as
//   basic_grouping_page does). The "On scrolling some items may disappear" note describes a C# native
//   rendering issue, not a behavior to reproduce.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class measure_first_strategy_page
    {
    public:
        // The reflection-free Member: just the Name the item template binds.
        struct member
        {
            std::string name;
            friend bool operator==(const member&, const member&) = default;
        };

        // The reflection-free Team KEY (the BindingContext of the group header/footer templates): Name
        // for {Binding Name} and Count for {Binding Count} (a build-time snapshot — see header note).
        struct team_key
        {
            std::string name;
            int count = 0;
            friend bool operator==(const team_key&, const team_key&) = default;
        };

        measure_first_strategy_page()
        {
            page_.set_title("MeasureFirstStrategy");
            root_.set_spacing(12);

            // ---- the explanatory Label (verbatim intent from XAML) ----
            info_.set_text("Using ItemSizingStrategy.MeasureFirstItem. On scrolling, some items may "
                           "disappear; this is a current known issue.");

            // ---- the strategy toggle + readout (makes the headline strategy observable headless) ----
            toggle_button_.set_text("Toggle Sizing Strategy");
            toggle_button_.clicked.connect([this] { toggle_strategy(); });

            // ---- the item template: a Label bound to Member.Name (StackLayout wrapper is cosmetic —
            // note) ----
            auto item_cell = maui::controls::data_template::of<maui::controls::label>();
            item_cell->set_binding<std::string, member>(maui::controls::label::text_property(),
                                                        [](const member& value) { return value.name; });
            list_.set_item_template(item_cell);

            // ---- the group header template: a LightGreen bold Label bound to Team.Name ----
            auto group_header = maui::controls::data_template::of<maui::controls::label>();
            group_header->set_binding<std::string, team_key>(maui::controls::label::text_property(),
                                                             [](const team_key& key) { return key.name; });
            group_header->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::light_green);
            list_.set_group_header_template(group_header);

            // ---- the group footer template: an Orange Label bound to Team.Count, formatted like
            // {Binding Count, StringFormat='Total members: {0:D}'} ----
            auto group_footer = maui::controls::data_template::of<maui::controls::label>();
            group_footer->set_binding<std::string, team_key>(
                maui::controls::label::text_property(),
                [](const team_key& key) { return "Total members: " + std::to_string(key.count); });
            group_footer->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::orange);
            list_.set_group_footer_template(group_footer);

            // ---- the headline: ItemSizingStrategy="MeasureFirstItem" + IsGrouped="True" ----
            list_.set_item_sizing_strategy(strategy_); // XAML MeasureFirstItem
            list_.set_is_grouped(true);
            list_.set_items_source(build_teams()); // ItemsSource = new SuperTeams()

            update_readout();

            root_.add(info_);
            root_.add(toggle_button_);
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
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::item_sizing_strategy strategy() const
        {
            return strategy_;
        }

        // Flip MeasureFirstItem ↔ MeasureAllItems over the same grouped content (the strategy is the
        // measure-pass policy the virtualization sim keys on).
        void toggle_strategy()
        {
            strategy_ = (strategy_ == maui::controls::item_sizing_strategy::measure_first_item)
                            ? maui::controls::item_sizing_strategy::measure_all_items
                            : maui::controls::item_sizing_strategy::measure_first_item;
            list_.set_item_sizing_strategy(strategy_);
            update_readout();
        }

    private:
        [[nodiscard]] static const char* strategy_name(maui::controls::item_sizing_strategy value)
        {
            return value == maui::controls::item_sizing_strategy::measure_first_item ? "MeasureFirstItem"
                                                                                     : "MeasureAllItems";
        }

        void update_readout()
        {
            readout_.set_text(std::string{"ItemSizingStrategy: "} + strategy_name(strategy_));
        }

        // SuperTeams(): the six Marvel rosters, each a grouping of a team_key (Name + member Count) over
        // an observable_collection<member>.
        [[nodiscard]] std::shared_ptr<maui::controls::i_item_collection> build_teams()
        {
            std::vector<maui::controls::grouping_ptr> teams;
            teams.push_back(
                make_team("Avengers", {"Thor", "Captain America", "Iron Man", "The Hulk", "Ant-Man", "Wasp", "Hawkeye",
                                       "Black Panther", "Black Widow", "Doctor Druid", "She-Hulk", "Mockingbird"}));
            teams.push_back(
                make_team("Fantastic Four", {"The Thing", "The Human Torch", "The Invisible Woman", "Mr. Fantastic"}));
            teams.push_back(make_team("Defenders", {"Doctor Strange", "Namor", "Hulk", "Silver Surfer", "Hellcat",
                                                    "Nighthawk", "Yellowjacket"}));
            teams.push_back(
                make_team("Heroes for Hire", {"Luke Cage", "Iron Fist", "Misty Knight", "Colleen Wing", "Shang-Chi"}));
            teams.push_back(
                make_team("West Coast Avengers", {"Hawkeye", "Mockingbird", "War Machine", "Wonder Man", "Tigra"}));
            teams.push_back(make_team("Great Lakes Avengers",
                                      {"Squirrel Girl", "Dinah Soar", "Mr. Immortal", "Flatman", "Doorman"}));

            // Keep the live collection alive for the page's lifetime so the grouped source stays valid.
            groups_ =
                std::make_shared<maui::core::observable_collection<maui::controls::grouping_ptr>>(std::move(teams));
            return maui::controls::make_item_collection(groups_);
        }

        // One Team: a team_key (Name + the roster Count) over the member collection.
        [[nodiscard]] static maui::controls::grouping_ptr make_team(std::string name, std::vector<std::string> names)
        {
            std::vector<member> members;
            members.reserve(names.size());
            for (std::string& member_name : names)
            {
                members.push_back(member{std::move(member_name)});
            }
            const int count = static_cast<int>(members.size());
            auto roster = std::make_shared<maui::core::observable_collection<member>>(std::move(members));
            return maui::controls::make_grouping(std::make_shared<team_key>(team_key{std::move(name), count}),
                                                 std::move(roster));
        }

        maui::controls::item_sizing_strategy strategy_ =
            maui::controls::item_sizing_strategy::measure_first_item; // XAML ItemSizingStrategy="MeasureFirstItem"
        std::shared_ptr<maui::core::observable_collection<maui::controls::grouping_ptr>>
            groups_; // pins the live source
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::label info_;
        maui::controls::button toggle_button_;
        maui::controls::label readout_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
