#pragma once
// maui::samples::switch_grouping_page — ports CollectionViewGalleries/GroupingGalleries/
// SwitchGrouping.xaml (+ .xaml.cs) of the C# CollectionView gallery.
//
// The original page (SwitchGrouping): a StackLayout with
//   - a horizontal StackLayout > Label "Is Grouped:" + a Switch (x:Name=GroupingSwitch) whose
//     BindingContext is {x:Reference CollectionView} and whose IsToggled is {Binding IsGrouped} — i.e.
//     the Switch is two-way bound to the CollectionView's IsGrouped, so flipping it toggles grouping
//     LIVE on the same source;
//   - a CollectionView (x:Name=CollectionView) with three DataTemplates:
//       ItemTemplate        → a StackLayout > Label bound to {Binding Name}   (each Member.Name);
//       GroupHeaderTemplate → a LightGreen bold Label bound to {Binding Name} (each Team.Name);
//       GroupFooterTemplate → an Orange Label bound to {Binding Count, StringFormat='Total members: {0:D}'}.
//     The code-behind sets CollectionView.ItemsSource = new SuperTeams() — a List<Team>, Team :
//     List<Member> { Name }. So the SAME source is read flat (ungrouped) or as groups depending on the
//     IsGrouped toggle; the gallery's point is watching the group headers/footers appear/disappear as
//     the Switch flips.
//
// The port models grouping structurally (item_collection.hpp): a grouped source is an
// item_collection<grouping_ptr>, each grouping carrying a KEY object (the BindingContext the group
// header/footer templates bind against) plus the nested member collection. The reflection-free port
// can't bind C#'s List<T>.Count off the Team object, so the key carries an explicit `count` snapshot
// captured at build time — the footer binds that (the documented stand-in for {Binding Count}). This
// mirrors basic_grouping_page / grid_grouping_page exactly.
//
//   - member: the reflection-free Member (just Name);
//   - team_key: the reflection-free Team KEY half — Name (group header) + Count (group footer);
//   - build_teams() reproduces SuperTeams() (the six Marvel rosters) as make_grouping of a team_key
//     over an observable_collection<member>;
//   - the item / group-header / group-footer templates mirror the three XAML DataTemplates;
//   - a toggle_switch (the port of Switch — `switch` is a C++ keyword) stands in for GroupingSwitch:
//     its `toggled` event writes list_.set_is_grouped(value), the code-first expression of the XAML
//     two-way {Binding IsGrouped} on the {x:Reference CollectionView} context. is_grouped starts true
//     (the switch starts On) so the page opens grouped, like the live source. toggle_grouping() /
//     set_grouping below drive the same path headlessly.
//
// note: the bound XAML wires the switch BOTH ways (the CollectionView's IsGrouped also writes back to
//       the switch). Here the switch is the sole driver of IsGrouped (one-way switch→list), which is
//       all the gallery exercises (the user only ever flips the switch); IsGrouped is not changed from
//       anywhere else, so the two-way back-channel has no observable effect to reproduce.
//
// The headless collection_view virtualization sim realizes per-section group header/footer
// supplementals with the group KEY as their binding context when IsGrouped is true (group names,
// member rows, "Total members: N" footers); flipping IsGrouped off reads the same source flat.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class switch_grouping_page
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

        switch_grouping_page()
        {
            page_.set_title("Switch Grouping");
            outer_.set_spacing(4);

            // ---- the "Is Grouped:" row: a horizontal StackLayout > Label + Switch ----
            grouped_label_.set_text("Is Grouped:");
            grouping_switch_.set_is_toggled(true); // the page opens grouped (IsGrouped starts true)
            switch_connection_ = maui::core::connect_scoped(grouping_switch_.toggled, [this](bool toggled) {
                list_.set_is_grouped(toggled);
            }); // the {Binding IsGrouped} write

            row_.set_orientation(maui::controls::stack_orientation::horizontal);
            row_.set_spacing(8);
            row_.add(grouped_label_);
            row_.add(grouping_switch_);

            // ---- the item template: a Label bound to Member.Name (the StackLayout wrapper has no
            // headless-visible effect, so we bind the label directly — note:) ----
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

            // ---- IsGrouped follows the switch (starts true); ItemsSource = new SuperTeams() ----
            list_.set_is_grouped(grouping_switch_.is_toggled());
            list_.set_items_source(build_teams());

            outer_.add(row_);
            outer_.add(list_);
            page_.set_content(outer_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- headless drivers (the switch path, observable) ----
        // Flip the switch (and thus IsGrouped) — the gallery's sole interaction.
        void set_grouping(bool grouped)
        {
            grouping_switch_.set_is_toggled(grouped); // raises `toggled` → list_.set_is_grouped
        }
        void toggle_grouping()
        {
            set_grouping(!grouping_switch_.is_toggled());
        }

        // ---- accessors (used by the hosting main + any test tree) ----
        [[nodiscard]] maui::controls::toggle_switch& grouping_switch()
        {
            return grouping_switch_;
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

        std::shared_ptr<maui::core::observable_collection<maui::controls::grouping_ptr>>
            groups_; // pins the live source
        maui::controls::content_page page_;
        maui::controls::stack_layout outer_;
        maui::controls::stack_layout row_;
        maui::controls::label grouped_label_;
        maui::controls::toggle_switch grouping_switch_;
        maui::controls::collection_view list_;
        maui::core::scoped_connection switch_connection_;
    };
} // namespace maui::samples
