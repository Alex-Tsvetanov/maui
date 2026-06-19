#pragma once
// maui::samples::some_empty_groups_page — ports GroupingGalleries/SomeEmptyGroups.xaml (+ .xaml.cs).
//
// The original page (SomeEmptyGroups): a StackLayout containing a description Label and, below it, a
// grouped CollectionView (IsGrouped="True", the default linear layout, NO view-level Header/Footer).
// Its point is the EMPTY-GROUP case: the source is built inline (in the xaml.cs ctor) as a List<Team>
// where some teams have an EMPTY member list, and the page documents that those empty groups should
// STILL display their group header/footer (group supplementals render even with zero items). The
// three DataTemplates are the same shape as the other grouping galleries:
//   - ItemTemplate        → a StackLayout > Label bound to {Binding Name}   (each Member.Name);
//   - GroupHeaderTemplate → a LightGreen bold Label bound to {Binding Name} (each Team.Name);
//   - GroupFooterTemplate → an Orange Label bound to {Binding Count, StringFormat='Total members: {0:D}'}.
//
// The inline C# source (xaml.cs) is exactly:
//   Avengers       → [Thor, Captain America]
//   Thundercats    → []                         (empty group)
//   Avengers       → [Thor, Captain America]    (a deliberate duplicate name)
//   Bionic Six     → []                         (empty group)
//   Fantastic Four → [The Thing, The Human Torch, The Invisible Woman, Mr. Fantastic]
// This port reproduces that list verbatim (including the duplicate "Avengers" and the two zero-member
// groups) so the empty-group headers/footers are exercised.
//
// The description Label's text comes from an OnPlatform in XAML (a default string + an iOS-specific
// variant noting an old-iOS clumping issue). The headless port has no platform switch at the sample
// layer, so the Label carries the DEFAULT string (note:) — the iOS caveat is platform commentary, not
// behavior.
//
// Grouping is modeled structurally (item_collection.hpp), identical to basic_grouping_page: a grouped
// source is an item_collection<grouping_ptr>, each grouping carrying a KEY object (the BindingContext
// the group header/footer templates bind against) plus the nested member collection. The reflection-
// free port can't bind C#'s List<T>.Count off the Team object, so the key carries an explicit `count`
// snapshot captured at build time — the footer binds that (the documented {Binding Count} stand-in).
// For the empty groups that count is 0, so their footers read "Total members: 0".
//
// The page OWNS its whole element tree (the items_page pattern); attach_handlers wires handlers
// bottom-up then re-hosts the stack + page. The headless collection_view virtualization sim realizes
// per-section group header/footer supplementals with the group KEY as their binding context — so a
// static capture shows EVERY group's name + "Total members: N" footer, including the empty ones, which
// is exactly the behavior the oracle page is demonstrating.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class some_empty_groups_page
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

        some_empty_groups_page()
        {
            page_.set_title("Some Empty Groups");
            stack_.set_spacing(8);

            // ---- the description Label (the OnPlatform Default string; see header note) ----
            description_.set_text("The CollectionView below should be grouped and some of the groups should be empty, "
                                  "but still display headers/footers.");

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
            // {Binding Count, StringFormat='Total members: {0:D}'} (reads "Total members: 0" for the
            // empty groups) ----
            auto group_footer = maui::controls::data_template::of<maui::controls::label>();
            group_footer->set_binding<std::string, team_key>(
                maui::controls::label::text_property(),
                [](const team_key& key) { return "Total members: " + std::to_string(key.count); });
            group_footer->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::orange);
            list_.set_group_footer_template(group_footer);

            // ---- IsGrouped (no view-level Header/Footer on this page) + the inline source ----
            list_.set_is_grouped(true);
            list_.set_items_source(build_teams());

            stack_.add(description_);
            stack_.add(list_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, description_, "description_");
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_); // the stack hosts the description label + collection view
            gallery_rehost_content(page_); // the page hosts the stack
        }

        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::label& description()
        {
            return description_;
        }

    private:
        // The inline xaml.cs source: five teams, two of which (Thundercats, Bionic Six) are EMPTY —
        // reproduced verbatim, including the duplicate "Avengers" name.
        [[nodiscard]] std::shared_ptr<maui::controls::i_item_collection> build_teams()
        {
            std::vector<maui::controls::grouping_ptr> teams;
            teams.push_back(make_team("Avengers", {"Thor", "Captain America"}));
            teams.push_back(make_team("Thundercats", {}));                       // empty group
            teams.push_back(make_team("Avengers", {"Thor", "Captain America"})); // deliberate duplicate
            teams.push_back(make_team("Bionic Six", {}));                        // empty group
            teams.push_back(
                make_team("Fantastic Four", {"The Thing", "The Human Torch", "The Invisible Woman", "Mr. Fantastic"}));

            // Keep the live collection alive for the page's lifetime so the grouped source stays valid.
            groups_ =
                std::make_shared<maui::core::observable_collection<maui::controls::grouping_ptr>>(std::move(teams));
            return maui::controls::make_item_collection(groups_);
        }

        // One Team: a team_key (Name + the roster Count) over the member collection (possibly empty).
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
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label description_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
