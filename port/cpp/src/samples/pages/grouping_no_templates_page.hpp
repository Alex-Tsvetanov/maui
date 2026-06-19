#pragma once
// maui::samples::grouping_no_templates_page — ports GroupingGalleries/GroupingNoTemplates.xaml
// (+ .xaml.cs) of the C# CollectionView gallery.
//
// The original page is the minimal grouping case: a single CollectionView with IsGrouped="True" and
// NOTHING else — no ItemTemplate, no GroupHeaderTemplate, no GroupFooterTemplate, no Header/Footer.
//   <CollectionView x:Name="CollectionView" IsGrouped="True" />
// The xaml.cs ctor sets ItemsSource = new SuperTeams() — the same List<Team> (Team : List<Member>) used
// by BasicGrouping: six Marvel rosters keyed by Team.Name. With NO templates, the CollectionView falls
// back to its DEFAULT rendering of the grouped data: each item cell and each group boundary shows the
// object's ToString() (Team.ToString() = Name; Member.ToString() = Name). That is the demonstrated
// feature: IsGrouped with no group/item templates — grouped data rendered by the framework's defaults.
//
// Port mapping (mirrors basic_grouping_page's structural grouping, but stripped of all three templates +
// the view-level header/footer to match this oracle):
//   - member is the reflection-free Member — just Name, made STRING-CONVERTIBLE (operator std::string)
//     so the default (template-less) item cell renders it: with no item_template the handler binds
//     `cell.text = boxed_item.text()`, and boxed_item::text() is the ToString stand-in, which only
//     produces text for string-convertible Ts (boxed_item.hpp). This is exactly the C# Member.ToString();
//   - team_key is the reflection-free Team KEY half — Name, likewise string-convertible so the default
//     group-boundary cell renders the team name (the C# Team.ToString() = Name);
//   - build_teams() reproduces SuperTeams() (the same six rosters as BasicGrouping), each as a
//     make_grouping of a team_key + an observable_collection<member>;
//   - set_is_grouped(true) is the whole config; NO set_item_template / set_group_*_template /
//     set_header / set_footer calls (the oracle declares none).
//
// note: with no templates the boxed item must carry a ToString stand-in for the default cell to show
//       text; member/team_key therefore expose `operator std::string` returning Name. This mirrors the
//       C# ToString() overrides on Member/Team (ViewModel.cs) one-for-one — it is the framework's
//       documented default-rendering path, not invented behavior.
//
// The headless collection_view virtualization simulator realizes per-section group-boundary cells with
// the group KEY as the value and per-item cells with each member as the value; with no templates each
// renders value.text(), so a static capture shows the group names and the member rows by default.

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class grouping_no_templates_page
    {
    public:
        // The reflection-free Member: just Name. String-convertible so the template-less default item
        // cell renders it (the C# Member.ToString() = Name; see header note).
        struct member
        {
            std::string name;
            operator std::string() const // NOLINT(google-explicit-constructor) — the ToString stand-in
            {
                return name;
            }
            friend bool operator==(const member&, const member&) = default;
        };

        // The reflection-free Team KEY (the group's identity object): Name. String-convertible so the
        // template-less default group-boundary cell renders it (the C# Team.ToString() = Name).
        struct team_key
        {
            std::string name;
            operator std::string() const // NOLINT(google-explicit-constructor) — the ToString stand-in
            {
                return name;
            }
            friend bool operator==(const team_key&, const team_key&) = default;
        };

        grouping_no_templates_page()
        {
            page_.set_title("Grouping (no templates)");

            // The entire oracle config: IsGrouped, no templates, no header/footer.
            list_.set_is_grouped(true);

            // ItemsSource = new SuperTeams().
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
        // SuperTeams(): the six Marvel rosters, each a grouping of a team_key (Name) over an
        // observable_collection<member> (the same data as BasicGrouping / GroupingNoTemplates oracle).
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

        // One Team: a team_key (Name) over the member collection.
        [[nodiscard]] static maui::controls::grouping_ptr make_team(std::string name, std::vector<std::string> names)
        {
            std::vector<member> members;
            members.reserve(names.size());
            for (std::string& member_name : names)
            {
                members.push_back(member{std::move(member_name)});
            }
            auto roster = std::make_shared<maui::core::observable_collection<member>>(std::move(members));
            return maui::controls::make_grouping(std::make_shared<team_key>(team_key{std::move(name)}),
                                                 std::move(roster));
        }

        std::shared_ptr<maui::core::observable_collection<maui::controls::grouping_ptr>>
            groups_; // pins the live source
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
