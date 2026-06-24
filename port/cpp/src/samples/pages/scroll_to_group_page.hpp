#pragma once
// maui::samples::scroll_to_group_page — ports ScrollToGalleries/ScrollToGroup.xaml (+ .xaml.cs) of the
// C# CollectionView gallery.
//
// The original page (ScrollToGroup): a StackLayout holding
//   - Grid #1: a "Group:" Label + numeric Entry (GroupIndex, default "0"), an "Item:" Label + numeric
//     Entry (ItemIndex, default "0"), and a "Go" Button (ScrollTo) whose Clicked calls
//     CollectionView.ScrollTo(itemIndex, groupIndex)  — the {index, groupIndex} POSITION overload;
//   - Grid #2: a "Group Name:" Label + Entry (GroupName), an "Item Name:" Label + Entry (ItemName), and a
//     "Go" Button (ScrollToItem) whose Clicked looks up the Team by name and the Member by name and calls
//     CollectionView.ScrollTo(member, team) — the {item, group} ELEMENT overload;
//   - a grouped CollectionView (IsGrouped="True") over `new SuperTeams()` (List<Team> : List<Member>),
//     with an ItemTemplate (Label bound to Member.Name), a GroupHeaderTemplate (LightGreen bold Label
//     bound to Team.Name) and a GroupFooterTemplate (Orange Label bound to Team.Count "Total members: N").
//
// This port mirrors that shape code-first, reusing the basic_grouping_page grouped-source modeling:
//   - the grouped source is an item_collection<grouping_ptr>, each grouping carrying a `team_key`
//     (Name + a build-time Count snapshot — the reflection-free stand-in for {Binding Count}, exactly the
//     basic_grouping_page convention) over an observable_collection<member>;
//   - is_grouped + the item / group-header / group-footer templates mirror the three XAML DataTemplates;
//   - the POSITION "Go" button reads the two index entries and calls scroll_to(item_index, group_index)
//     (items_view::scroll_to(int, int) — the C# ScrollTo(itemIndex, groupIndex) position overload);
//   - the ELEMENT "Go" button looks the Team up by name then the Member up by name and calls
//     scroll_to(boxed member, boxed team) (items_view::scroll_to(boxed_item, boxed_item) — the C#
//     ScrollTo(member, team) element overload). A missing group is a no-op, exactly the C# `team == null`
//     early return; a missing member boxes the null item (C# FirstOrDefault → null), which scroll_to still
//     funnels (the C# element overload accepts a null item).
//
// readout: the ScrollTo request is observed through the collection_view's `scroll_to_requested` event —
// the port's faithful equivalent of the C# event ItemsView wires to its handler — and the readout label
// reports the requested target (position "group G / item I" or element "group '<name>' / item '<name>'").
// This is the demo affordance the task calls for; the oracle page has no such label (note:).
//
// The page OWNS its whole element tree (the items_page pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless
// collection_view virtualization sim realizes the grouped sections so a static capture shows the group
// names, member rows, and "Total members: N" footers, and ScrollTo requests surface through the readout.
//
// note: the C# group header/footer carry BackgroundColor (LightGreen / Orange). The port has no
//       view-level background setter on label (the basic_grouping_page precedent), so the coloring is
//       applied through text_color instead — the demonstrated cue (a colored group chrome) is preserved.
// note: the C# entries declare Keyboard="Numeric"; the port entry carries a keyboard property, but the
//       numeric restriction is a native-input concern with no headless effect, so it is left default here.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/scroll_to_request_event_args.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class scroll_to_group_page
    {
    public:
        // The reflection-free Member: just the Name the item template binds (basic_grouping_page::member).
        struct member
        {
            std::string name;
            friend bool operator==(const member&, const member&) = default;
        };

        // The reflection-free Team KEY (the BindingContext of the group header/footer templates): Name for
        // {Binding Name} and Count for {Binding Count} (a build-time snapshot — see header note).
        struct team_key
        {
            std::string name;
            int count = 0;
            friend bool operator==(const team_key&, const team_key&) = default;
        };

        scroll_to_group_page()
        {
            page_.set_title("ScrollTo Group");
            root_.set_spacing(12);

            build_index_grid(); // Grid #1: Group/Item index entries + the position "Go"
            build_name_grid();  // Grid #2: Group/Item name entries + the element "Go"
            build_collection(); // the grouped CollectionView

            readout_.set_text("No scroll requested yet");

            root_.add(index_grid_);
            root_.add(name_grid_);
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
        [[nodiscard]] maui::controls::button& scroll_to_button()
        {
            return scroll_to_button_;
        }
        [[nodiscard]] maui::controls::button& scroll_to_item_button()
        {
            return scroll_to_item_button_;
        }
        [[nodiscard]] maui::controls::entry& group_index_entry()
        {
            return group_index_entry_;
        }
        [[nodiscard]] maui::controls::entry& item_index_entry()
        {
            return item_index_entry_;
        }
        [[nodiscard]] maui::controls::entry& group_name_entry()
        {
            return group_name_entry_;
        }
        [[nodiscard]] maui::controls::entry& item_name_entry()
        {
            return item_name_entry_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

        // ScrollToClicked: CollectionView.ScrollTo(itemIndex, groupIndex) — the position overload.
        void scroll_to_by_index()
        {
            const int group_index = parse_int(std::string(group_index_entry_.text()), 0);
            const int item_index = parse_int(std::string(item_index_entry_.text()), 0);
            list_.scroll_to(item_index, group_index);
        }

        // ScrollToItemClicked: find the Team by name, the Member by name, ScrollTo(member, team) — the
        // element overload. A missing group is a no-op (C# `team == null` early return).
        void scroll_to_by_name()
        {
            const std::string group_name = std::string(group_name_entry_.text());
            const std::string item_name = std::string(item_name_entry_.text());

            const team_key* team = find_team(group_name);
            if (team == nullptr)
            {
                return;
            }
            // FirstOrDefault(member.Name == itemName): a found member boxes that member; a miss boxes the
            // null item (C# null member is still passed to the element overload).
            const maui::controls::boxed_item member_box = box_member(group_name, item_name);
            const maui::controls::boxed_item team_box = maui::controls::boxed_item::of<team_key>(*team);
            list_.scroll_to(member_box, team_box);
        }

    private:
        void build_index_grid()
        {
            index_grid_.add_row_definition(maui::core::grid_length::automatic());
            index_grid_.add_row_definition(maui::core::grid_length::automatic());
            index_grid_.add_row_definition(maui::core::grid_length::automatic());
            index_grid_.add_column_definition(maui::core::grid_length::star());
            index_grid_.add_column_definition(maui::core::grid_length::star());

            group_index_label_.set_text("Group:");
            item_index_label_.set_text("Item:");
            group_index_entry_.set_text("0"); // C# Text="0"
            item_index_entry_.set_text("0");
            scroll_to_button_.set_text("Go");
            scroll_to_button_.command = [this] { scroll_to_by_index(); };

            index_grid_.set_row(group_index_label_, 0);
            index_grid_.set_column(group_index_label_, 0);
            index_grid_.add(group_index_label_);
            index_grid_.set_row(group_index_entry_, 0);
            index_grid_.set_column(group_index_entry_, 1);
            index_grid_.add(group_index_entry_);
            index_grid_.set_row(item_index_label_, 1);
            index_grid_.set_column(item_index_label_, 0);
            index_grid_.add(item_index_label_);
            index_grid_.set_row(item_index_entry_, 1);
            index_grid_.set_column(item_index_entry_, 1);
            index_grid_.add(item_index_entry_);
            index_grid_.set_row(scroll_to_button_, 2);
            index_grid_.set_column(scroll_to_button_, 0);
            index_grid_.set_column_span(scroll_to_button_, 2);
            index_grid_.add(scroll_to_button_);
        }

        void build_name_grid()
        {
            name_grid_.add_row_definition(maui::core::grid_length::automatic());
            name_grid_.add_row_definition(maui::core::grid_length::automatic());
            name_grid_.add_row_definition(maui::core::grid_length::automatic());
            name_grid_.add_column_definition(maui::core::grid_length::star());
            name_grid_.add_column_definition(maui::core::grid_length::star());

            group_name_label_.set_text("Group Name:");
            item_name_label_.set_text("Item Name:");
            scroll_to_item_button_.set_text("Go");
            scroll_to_item_button_.command = [this] { scroll_to_by_name(); };

            name_grid_.set_row(group_name_label_, 0);
            name_grid_.set_column(group_name_label_, 0);
            name_grid_.add(group_name_label_);
            name_grid_.set_row(group_name_entry_, 0);
            name_grid_.set_column(group_name_entry_, 1);
            name_grid_.add(group_name_entry_);
            name_grid_.set_row(item_name_label_, 1);
            name_grid_.set_column(item_name_label_, 0);
            name_grid_.add(item_name_label_);
            name_grid_.set_row(item_name_entry_, 1);
            name_grid_.set_column(item_name_entry_, 1);
            name_grid_.add(item_name_entry_);
            name_grid_.set_row(scroll_to_item_button_, 2);
            name_grid_.set_column(scroll_to_item_button_, 0);
            name_grid_.set_column_span(scroll_to_item_button_, 2);
            name_grid_.add(scroll_to_item_button_);
        }

        void build_collection()
        {
            // ItemTemplate: a Label bound to Member.Name (the StackLayout wrapper has no headless-visible
            // effect, so we bind the label directly — basic_grouping_page convention).
            auto item_cell = maui::controls::data_template::of<maui::controls::label>();
            item_cell->set_binding<std::string, member>(maui::controls::label::text_property(),
                                                        [](const member& value) { return value.name; });
            list_.set_item_template(item_cell);

            // GroupHeaderTemplate: a LightGreen bold Label bound to Team.Name (color via text_color — note:).
            auto group_header = maui::controls::data_template::of<maui::controls::label>();
            group_header->set_binding<std::string, team_key>(maui::controls::label::text_property(),
                                                             [](const team_key& key) { return key.name; });
            group_header->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::light_green);
            list_.set_group_header_template(group_header);

            // GroupFooterTemplate: an Orange Label bound to Team.Count ("Total members: N").
            auto group_footer = maui::controls::data_template::of<maui::controls::label>();
            group_footer->set_binding<std::string, team_key>(
                maui::controls::label::text_property(),
                [](const team_key& key) { return "Total members: " + std::to_string(key.count); });
            group_footer->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::orange);
            list_.set_group_footer_template(group_footer);

            list_.set_is_grouped(true);
            list_.set_items_source(build_teams()); // ItemsSource = new SuperTeams()

            // Observe ScrollTo requests for the readout (the demo affordance — see header note).
            list_.scroll_to_requested.connect(
                [this](const maui::controls::scroll_to_request_event_args& args) { update_readout(args); });
        }

        void update_readout(const maui::controls::scroll_to_request_event_args& args)
        {
            if (args.mode == maui::controls::scroll_to_mode::position)
            {
                readout_.set_text("Requested position: group " + std::to_string(args.group_index) + " / item " +
                                  std::to_string(args.index));
            }
            else
            {
                const std::string item_name = args.item.has_value() ? args.item.text() : std::string{"(null)"};
                const std::string group_name = args.group.has_value() ? group_text(args.group) : std::string{"(null)"};
                readout_.set_text("Requested element: group '" + group_name + "' / item '" + item_name + "'");
            }
        }

        // The group box carries a team_key; render its Name (boxed_item::text has no reflection form for a
        // struct without operator<<, so read the key directly).
        [[nodiscard]] static std::string group_text(const maui::controls::boxed_item& group)
        {
            if (const std::shared_ptr<team_key> key = group.as<team_key>())
            {
                return key->name;
            }
            return {};
        }

        // SuperTeams(): the six Marvel rosters (basic_grouping_page::build_teams), each a grouping of a
        // team_key (Name + member Count) over an observable_collection<member>.
        [[nodiscard]] std::shared_ptr<maui::controls::i_item_collection> build_teams()
        {
            roster_names_ = {
                {"Avengers",
                 {"Thor", "Captain America", "Iron Man", "The Hulk", "Ant-Man", "Wasp", "Hawkeye", "Black Panther",
                  "Black Widow", "Doctor Druid", "She-Hulk", "Mockingbird"}},
                {"Fantastic Four", {"The Thing", "The Human Torch", "The Invisible Woman", "Mr. Fantastic"}},
                {"Defenders",
                 {"Doctor Strange", "Namor", "Hulk", "Silver Surfer", "Hellcat", "Nighthawk", "Yellowjacket"}},
                {"Heroes for Hire", {"Luke Cage", "Iron Fist", "Misty Knight", "Colleen Wing", "Shang-Chi"}},
                {"West Coast Avengers", {"Hawkeye", "Mockingbird", "War Machine", "Wonder Man", "Tigra"}},
                {"Great Lakes Avengers", {"Squirrel Girl", "Dinah Soar", "Mr. Immortal", "Flatman", "Doorman"}},
            };

            std::vector<maui::controls::grouping_ptr> teams;
            teams.reserve(roster_names_.size());
            for (const auto& roster : roster_names_)
            {
                teams.push_back(make_team(roster.first, roster.second));
            }
            groups_ =
                std::make_shared<maui::core::observable_collection<maui::controls::grouping_ptr>>(std::move(teams));
            return maui::controls::make_item_collection(groups_);
        }

        // One Team: a team_key (Name + roster Count) over the member collection.
        [[nodiscard]] static maui::controls::grouping_ptr make_team(const std::string& name,
                                                                    const std::vector<std::string>& names)
        {
            std::vector<member> members;
            members.reserve(names.size());
            for (const std::string& member_name : names)
            {
                members.push_back(member{member_name});
            }
            const int count = static_cast<int>(members.size());
            auto roster = std::make_shared<maui::core::observable_collection<member>>(std::move(members));
            return maui::controls::make_grouping(std::make_shared<team_key>(team_key{name, count}), std::move(roster));
        }

        // SuperTeams.FirstOrDefault(t.Name == groupName): the team_key for the named group, or null.
        [[nodiscard]] const team_key* find_team(const std::string& group_name) const
        {
            for (const auto& roster : roster_names_)
            {
                if (roster.first == group_name)
                {
                    static thread_local team_key key;
                    key = team_key{roster.first, static_cast<int>(roster.second.size())};
                    return &key;
                }
            }
            return nullptr;
        }

        // team.FirstOrDefault(member.Name == itemName): box the found member, or the null item on a miss.
        [[nodiscard]] maui::controls::boxed_item box_member(const std::string& group_name,
                                                            const std::string& item_name) const
        {
            for (const auto& roster : roster_names_)
            {
                if (roster.first != group_name)
                {
                    continue;
                }
                for (const std::string& candidate : roster.second)
                {
                    if (candidate == item_name)
                    {
                        return maui::controls::boxed_item::of<member>(member{candidate});
                    }
                }
            }
            return {}; // null member (C# FirstOrDefault → null)
        }

        // int.Parse with a fallback (the C# int.Parse would throw on bad input; the demo tolerates it).
        [[nodiscard]] static int parse_int(const std::string& text, int fallback)
        {
            try
            {
                if (text.empty())
                {
                    return fallback;
                }
                return std::stoi(text);
            }
            catch (...)
            {
                return fallback;
            }
        }

        // The unfiltered roster definitions (group name → member names), the source for lookups.
        std::vector<std::pair<std::string, std::vector<std::string>>> roster_names_;
        std::shared_ptr<maui::core::observable_collection<maui::controls::grouping_ptr>>
            groups_; // pins the live grouped source

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;

        // Grid #1 — index entries + position "Go".
        maui::controls::grid index_grid_;
        maui::controls::label group_index_label_;
        maui::controls::entry group_index_entry_;
        maui::controls::label item_index_label_;
        maui::controls::entry item_index_entry_;
        maui::controls::button scroll_to_button_;

        // Grid #2 — name entries + element "Go".
        maui::controls::grid name_grid_;
        maui::controls::label group_name_label_;
        maui::controls::entry group_name_entry_;
        maui::controls::label item_name_label_;
        maui::controls::entry item_name_entry_;
        maui::controls::button scroll_to_item_button_;

        maui::controls::label readout_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
