#pragma once
// maui::samples::grouping_plus_selection_page — ports CollectionViewGalleries/GroupingGalleries/
// GroupingPlusSelection.xaml (+ .xaml.cs) of the C# CollectionView gallery.
//
// The original page (GroupingPlusSelection): a single CollectionView (x:Name=CollectionView) with BOTH
// IsGrouped="True" AND SelectionMode="Single", and the three DataTemplates:
//   - ItemTemplate        → a StackLayout > Label bound to {Binding Name}   (each Member.Name);
//   - GroupHeaderTemplate → a LightGreen bold Label bound to {Binding Name} (each Team.Name);
//   - GroupFooterTemplate → an Orange Label bound to {Binding Count, StringFormat='Total members: {0:D}'}.
// The code-behind sets CollectionView.ItemsSource = new SuperTeams() — a List<Team>, Team :
// List<Member> { Name }. The gallery's point is exercising grouping and single selection TOGETHER:
// tapping a member row selects that one member within its group while the group headers/footers stay.
//
// This is the grouping siblings (basic_grouping_page / grid_grouping_page) PLUS single selection — the
// only additions vs BasicGrouping are SelectionMode="Single" (and the absence of the view-level
// Header/Footer strings, which GroupingPlusSelection does not set). The port models grouping
// structurally (item_collection.hpp): a grouped source is an item_collection<grouping_ptr>, each
// grouping carrying a KEY object (the group header/footer BindingContext) plus the nested member
// collection. The reflection-free port can't bind C#'s List<T>.Count off the Team object, so the key
// carries an explicit `count` snapshot captured at build time — the footer binds that (the documented
// stand-in for {Binding Count}).
//
//   - member: the reflection-free Member (just Name);
//   - team_key: the reflection-free Team KEY half — Name (group header) + Count (group footer);
//   - build_teams() reproduces SuperTeams() (the six Marvel rosters) as make_grouping of a team_key
//     over an observable_collection<member>;
//   - is_grouped is true; selection_mode is Single (the selectable_items_view surface);
//   - the three templates mirror the XAML DataTemplates;
//   - select_member() / selected_member() drive + read the single selection headlessly (selection is
//     fully observable through selected_item(), even though the selected ROW has no headless highlight).
//
// note: GroupingPlusSelection sets no view-level Header/Footer (unlike BasicGrouping/GridGrouping), so
//       this port sets neither — only the per-GROUP header/footer templates.
//
// The headless collection_view virtualization sim realizes per-section group header/footer
// supplementals with the group KEY as their binding context (group names, member rows, "Total
// members: N" footers); single selection is exercised through set_selected_item / selected_item.

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/font.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class grouping_plus_selection_page
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

        grouping_plus_selection_page()
        {
            page_.set_title("Grouping Plus Selection");

            // ---- the item template: a Label bound to Member.Name (the StackLayout wrapper has no
            // headless-visible effect, so we bind the label directly — note:) ----
            auto item_cell = maui::controls::data_template::of<maui::controls::label>();
            item_cell->set_binding<std::string, member>(maui::controls::label::text_property(),
                                                        [](const member& value) { return value.name; });
            list_.set_item_template(item_cell);

            // ---- the group header template: a LightGreen bold Label bound to Team.Name ----
            // FontAttributes="Bold" (XAML) → the bold system font; folded into the font's weight
            // (FontExtensions.WithAttributes: Bold → font_weight::bold). Without this the header rendered
            // regular weight vs MAUI's bold (the "bold group headers render regular" Android parity diff).
            // No explicit FontSize (system default), matching the maui-compare reference for this page.
            auto group_header = maui::controls::data_template::of<maui::controls::label>();
            group_header->set_binding<std::string, team_key>(maui::controls::label::text_property(),
                                                             [](const team_key& key) { return key.name; });
            group_header->set_value(
                maui::controls::background_property(),
                std::static_pointer_cast<maui::graphics::paint>(
                    std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_green)));
            group_header->set_value(maui::controls::label::font_property(),
                                    maui::core::font::system_font_of_size(16, maui::core::font_weight::bold));
            list_.set_group_header_template(group_header);

            // ---- the group footer template: an Orange Label bound to Team.Count, formatted like
            // {Binding Count, StringFormat='Total members: {0:D}'} ----
            auto group_footer = maui::controls::data_template::of<maui::controls::label>();
            group_footer->set_binding<std::string, team_key>(
                maui::controls::label::text_property(),
                [](const team_key& key) { return "Total members: " + std::to_string(key.count); });
            group_footer->set_value(maui::controls::background_property(),
                                    std::static_pointer_cast<maui::graphics::paint>(
                                        std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::orange)));
            group_footer->set_value(maui::controls::margin_property(), maui::core::thickness(0, 0, 0, 15));
            list_.set_group_footer_template(group_footer);

            // ---- IsGrouped="True" + SelectionMode="Single" (the two features exercised together) ----
            list_.set_is_grouped(true);
            list_.set_selection_mode(maui::controls::selection_mode::single);

            // ---- ItemsSource = new SuperTeams() ----
            list_.set_items_source(build_teams());

            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- headless single-selection driver (selection is observable; the selected-row highlight is
        // not — the gallery's combined grouping+selection is exercised via selected_item) ----
        // Select one member in group `group_index` at member `member_index`.
        void select_member(std::size_t group_index, std::size_t member_index)
        {
            if (group_index < rosters_.size())
            {
                const auto& roster = *rosters_[group_index];
                if (member_index < roster.size())
                {
                    list_.set_selected_item(maui::controls::boxed_item::of(roster.at(member_index)));
                }
            }
        }
        // The currently selected member's Name (empty when nothing is selected).
        [[nodiscard]] std::string selected_member() const
        {
            if (const auto value = list_.selected_item().as<member>())
            {
                return value->name;
            }
            return {};
        }

        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }

    private:
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

        // One Team: a team_key (Name + the roster Count) over the member collection. The roster is also
        // pinned in rosters_ so the single-selection driver can address members by group/member index.
        [[nodiscard]] maui::controls::grouping_ptr make_team(std::string name, std::vector<std::string> names)
        {
            std::vector<member> members;
            members.reserve(names.size());
            for (std::string& member_name : names)
            {
                members.push_back(member{std::move(member_name)});
            }
            const int count = static_cast<int>(members.size());
            auto roster = std::make_shared<maui::core::observable_collection<member>>(std::move(members));
            rosters_.push_back(roster);
            return maui::controls::make_grouping(std::make_shared<team_key>(team_key{std::move(name), count}), roster);
        }

        std::shared_ptr<maui::core::observable_collection<maui::controls::grouping_ptr>>
            groups_; // pins the live source
        std::vector<std::shared_ptr<maui::core::observable_collection<member>>>
            rosters_; // per-group rosters, for the selection driver
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
