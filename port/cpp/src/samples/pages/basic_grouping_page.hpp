#pragma once
// maui::samples::basic_grouping_page — ports GroupingGalleries/BasicGrouping.xaml (+ .xaml.cs) of the
// C# CollectionView gallery.
//
// The original page (BasicGrouping): one CollectionView with IsGrouped="True", a view-level
// Header="This is a header" + Footer="Hey, a footer.", and three DataTemplates:
//   - ItemTemplate        → a StackLayout > Label bound to {Binding Name}   (each Member.Name);
//   - GroupHeaderTemplate → a LightGreen bold Label bound to {Binding Name} (each Team.Name);
//   - GroupFooterTemplate → an Orange Label bound to {Binding Count, StringFormat='Total members: {0}'}.
// ItemsSource = new SuperTeams() — a List<Team>, where Team : List<Member> { string Name }. So the
// grouped source is "an IEnumerable of IEnumerables", each group keyed by the Team object (whose Name
// the header binds and whose Count the footer binds).
//
// The port models grouping structurally (item_collection.hpp): a grouped source is an
// item_collection<grouping_ptr>, each grouping carrying a KEY object (the BindingContext the group
// header/footer templates bind against) plus the nested member collection. The reflection-free port
// can't bind C#'s List<T>.Count off the Team object, so the key carries an explicit `count` snapshot
// captured at build time — the footer binds that (the documented stand-in for {Binding Count}).
//
//   - member: the reflection-free Member (just Name);
//   - team_key: the reflection-free Team KEY half — Name (group header) + Count (group footer). The
//     "list half" of C#'s Team is the grouping's nested member collection;
//   - build_teams() reproduces SuperTeams() (the same six Marvel rosters), each as a make_grouping of
//     a team_key + an observable_collection<member>;
//   - is_grouped + the item/group-header/group-footer templates mirror the three XAML DataTemplates;
//   - Header/Footer are the view-level boxed strings.
//
// The headless collection_view virtualization sim realizes per-section group header/footer
// supplementals with the group KEY as their binding context, so a static capture shows the group
// names, the member rows, and the "Total members: N" footers.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class basic_grouping_page
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

        basic_grouping_page()
        {
            page_.set_title("Basic Grouping");

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
            // FontAttributes.Bold (default size) — maui-compare BasicGroupingPage's GroupHeaderTemplate sets
            // it; the port had only the color, so the green group headers rendered regular weight.
            group_header->set_value(maui::controls::label::font_property(),
                                    maui::core::font::system_font_of_weight(maui::core::font_weight::bold));
            list_.set_group_header_template(group_header);

            // ---- the group footer template: an Orange Label bound to Team.Count, formatted like
            // {Binding Count, StringFormat='Total members: {0:D}'} ----
            auto group_footer = maui::controls::data_template::of<maui::controls::label>();
            group_footer->set_binding<std::string, team_key>(
                maui::controls::label::text_property(),
                [](const team_key& key) { return "Total members: " + std::to_string(key.count); });
            group_footer->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::orange);
            list_.set_group_footer_template(group_footer);

            // ---- IsGrouped + the view-level header/footer strings ----
            list_.set_is_grouped(true);
            list_.set_header(maui::controls::boxed_item::of(std::string{"This is a header"}));
            list_.set_footer(maui::controls::boxed_item::of(std::string{"Hey, a footer."}));

            // ---- ItemsSource = new SuperTeams() ----
            list_.set_items_source(build_teams());

            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_content(page_); // the page hosts the grouped collection view
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
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
