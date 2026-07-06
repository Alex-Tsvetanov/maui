#pragma once
// super_teams.hpp — bindable grouped data for the gallery_xaml code-behind of the CollectionView
// grouping pages (basic_grouping.xaml, grid_grouping.xaml, switch_grouping.xaml, ...).
//
// WHY THIS EXISTS: real MAUI assigns these pages' grouped ItemsSource in code-behind
// (`CollectionView.ItemsSource = new SuperTeams()` — see port/maui-reference/app/Pages/*Page.xaml.cs),
// NOT in the shared XAML markup. The port's compile-time-XAML app (gallery_xaml) has no C# code-behind,
// so a hand-written {name}.xaml.cpp "code-behind" wires the data instead (find the x:Name'd CollectionView
// in the hydrated tree, set its items_source to super_teams()).
//
// The shared XAML's GroupHeaderTemplate/GroupFooterTemplate/ItemTemplate bind STRING PATHS
// ({Binding Name}, {Binding Count}), so — unlike the code-first gallery builder twin, which binds typed
// C++ lambdas over plain structs — the data items here must be bindable_objects that REGISTER those
// property names (boxed_item::of stores a shared_ptr<bindable_object> item as bindable, so the template's
// binding context resolves {Binding Name}/{Binding Count} against it). team_member carries Name (the item
// template); team carries Name (group header) + a build-time Count snapshot (group footer, the reflection-
// free stand-in for C#'s {Binding Count} off List<T>.Count — same convention as the builder twin).

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/items/item_collection.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/observable.hpp"
#include "maui/core/observable_collection.hpp"

namespace examples::ViewModels
{
    // One roster entry — the ItemTemplate binds {Binding Name}.
    class team_member : public maui::core::bindable_object
    {
    public:
        explicit team_member(std::string name)
        {
            Name.set(std::move(name));
        }
        maui::core::observable<std::string> Name{*this, "Name"};
    };

    // One team (the group key) — the GroupHeaderTemplate binds {Binding Name}, the GroupFooterTemplate
    // binds {Binding Count} (formatted 'Total members: {0}').
    class team : public maui::core::bindable_object
    {
    public:
        team(std::string name, int count)
        {
            Name.set(std::move(name));
            Count.set(count);
        }
        maui::core::observable<std::string> Name{*this, "Name"};
        maui::core::observable<int> Count{*this, "Count"};
    };

    // One group: a team key over its bindable member roster (Count snapshot = the roster size).
    [[nodiscard]] inline maui::controls::grouping_ptr make_team(std::string name, std::vector<std::string> names)
    {
        std::vector<std::shared_ptr<team_member>> members;
        members.reserve(names.size());
        for (std::string& member_name : names)
        {
            members.push_back(std::make_shared<team_member>(std::move(member_name)));
        }
        const int count = static_cast<int>(members.size());
        auto roster =
            std::make_shared<maui::core::observable_collection<std::shared_ptr<team_member>>>(std::move(members));
        return maui::controls::make_grouping(std::make_shared<team>(std::move(name), count), std::move(roster));
    }

    // SuperTeams(): the six Marvel rosters as a grouped i_item_collection. Reproduces the C# SuperTeams()
    // assigned by BasicGroupingPage.xaml.cs et al. The returned item_collection owns the observable_collection
    // (which owns the groupings and their members), so the whole source stays alive as long as the
    // CollectionView holds it.
    [[nodiscard]] inline std::shared_ptr<maui::controls::i_item_collection> super_teams()
    {
        std::vector<maui::controls::grouping_ptr> teams;
        teams.push_back(
            make_team("Avengers", {"Thor", "Captain America", "Iron Man", "The Hulk", "Ant-Man", "Wasp", "Hawkeye",
                                   "Black Panther", "Black Widow", "Doctor Druid", "She-Hulk", "Mockingbird"}));
        teams.push_back(
            make_team("Fantastic Four", {"The Thing", "The Human Torch", "The Invisible Woman", "Mr. Fantastic"}));
        teams.push_back(make_team(
            "Defenders", {"Doctor Strange", "Namor", "Hulk", "Silver Surfer", "Hellcat", "Nighthawk", "Yellowjacket"}));
        teams.push_back(
            make_team("Heroes for Hire", {"Luke Cage", "Iron Fist", "Misty Knight", "Colleen Wing", "Shang-Chi"}));
        teams.push_back(
            make_team("West Coast Avengers", {"Hawkeye", "Mockingbird", "War Machine", "Wonder Man", "Tigra"}));
        teams.push_back(
            make_team("Great Lakes Avengers", {"Squirrel Girl", "Dinah Soar", "Mr. Immortal", "Flatman", "Doorman"}));

        auto groups =
            std::make_shared<maui::core::observable_collection<maui::controls::grouping_ptr>>(std::move(teams));
        return maui::controls::make_item_collection(std::move(groups));
    }

    // grouping_no_templates has NO ItemTemplate / GroupHeaderTemplate, so the CollectionView renders each
    // group key and member via its text() (the ToString stand-in) — a bindable team/team_member exposes no
    // text(), so that page uses these string-convertible value types instead (operator std::string), exactly
    // like the code-first builder twin (grouping_no_templates_page.hpp).
    struct text_member
    {
        std::string name;
        operator std::string() const // NOLINT(google-explicit-constructor) — the ToString stand-in
        {
            return name;
        }
    };
    struct text_team
    {
        std::string name;
        operator std::string() const // NOLINT(google-explicit-constructor) — the ToString stand-in
        {
            return name;
        }
    };

    [[nodiscard]] inline maui::controls::grouping_ptr make_text_team(std::string name, std::vector<std::string> names)
    {
        std::vector<text_member> members;
        members.reserve(names.size());
        for (std::string& member_name : names)
        {
            members.push_back(text_member{std::move(member_name)});
        }
        auto roster = std::make_shared<maui::core::observable_collection<text_member>>(std::move(members));
        return maui::controls::make_grouping(std::make_shared<text_team>(text_team{std::move(name)}),
                                             std::move(roster));
    }

    // SuperTeams() as string-convertible value types for the template-less grouping page.
    [[nodiscard]] inline std::shared_ptr<maui::controls::i_item_collection> super_teams_text()
    {
        std::vector<maui::controls::grouping_ptr> teams;
        teams.push_back(
            make_text_team("Avengers", {"Thor", "Captain America", "Iron Man", "The Hulk", "Ant-Man", "Wasp", "Hawkeye",
                                        "Black Panther", "Black Widow", "Doctor Druid", "She-Hulk", "Mockingbird"}));
        teams.push_back(
            make_text_team("Fantastic Four", {"The Thing", "The Human Torch", "The Invisible Woman", "Mr. Fantastic"}));
        teams.push_back(make_text_team(
            "Defenders", {"Doctor Strange", "Namor", "Hulk", "Silver Surfer", "Hellcat", "Nighthawk", "Yellowjacket"}));
        teams.push_back(
            make_text_team("Heroes for Hire", {"Luke Cage", "Iron Fist", "Misty Knight", "Colleen Wing", "Shang-Chi"}));
        teams.push_back(
            make_text_team("West Coast Avengers", {"Hawkeye", "Mockingbird", "War Machine", "Wonder Man", "Tigra"}));
        teams.push_back(make_text_team("Great Lakes Avengers",
                                       {"Squirrel Girl", "Dinah Soar", "Mr. Immortal", "Flatman", "Doorman"}));

        auto groups =
            std::make_shared<maui::core::observable_collection<maui::controls::grouping_ptr>>(std::move(teams));
        return maui::controls::make_item_collection(std::move(groups));
    }

    // some_empty_groups' variant source: five teams, two of them empty (Thundercats, Bionic Six) — the
    // page demonstrates that empty groups still show their header/footer. Mirrors SomeEmptyGroupsPage.xaml.cs.
    [[nodiscard]] inline std::shared_ptr<maui::controls::i_item_collection> some_teams()
    {
        std::vector<maui::controls::grouping_ptr> teams;
        teams.push_back(make_team("Avengers", {"Thor", "Captain America"}));
        teams.push_back(make_team("Thundercats", {}));
        teams.push_back(make_team("Avengers", {"Thor", "Captain America"}));
        teams.push_back(make_team("Bionic Six", {}));
        teams.push_back(
            make_team("Fantastic Four", {"The Thing", "The Human Torch", "The Invisible Woman", "Mr. Fantastic"}));

        auto groups =
            std::make_shared<maui::core::observable_collection<maui::controls::grouping_ptr>>(std::move(teams));
        return maui::controls::make_item_collection(std::move(groups));
    }
} // namespace examples::ViewModels
