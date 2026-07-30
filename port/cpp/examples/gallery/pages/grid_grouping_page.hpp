#pragma once
// maui::samples::grid_grouping_page — ports GroupingGalleries/GridGrouping.xaml (+ .xaml.cs) of the
// C# CollectionView gallery.
//
// The original page (GridGrouping): one CollectionView with IsGrouped="True", a view-level
// Header="This is a header" + Footer="This is a footer.", an ItemsLayout that is a
// GridItemsLayout (Span="2" HorizontalItemSpacing="5" Orientation="Vertical") — i.e. the grouped
// rows lay out two-per-line in a vertical-scrolling grid — and three DataTemplates:
//   - ItemTemplate        → a StackLayout > Label bound to {Binding Name}   (each Member.Name);
//   - GroupHeaderTemplate → a LightGreen bold Label bound to {Binding Name} (each Team.Name);
//   - GroupFooterTemplate → a StackLayout > Orange Label (Margin="0,0,0,15") bound to {Binding Count,
//     StringFormat='Total members: {0:D}'}.
// ItemsSource = new SuperTeams() — a List<Team>, where Team : List<Member> { string Name }. So the
// grouped source is "an IEnumerable of IEnumerables", each group keyed by the Team object (whose Name
// the header binds and whose Count the footer binds).
//
// This is the sibling of basic_grouping_page; the ONLY structural difference vs BasicGrouping is the
// explicit GridItemsLayout (BasicGrouping uses the default linear layout) and the footer string
// ("This is a footer." here vs "Hey, a footer." there). Everything else — the three templates, the
// IsGrouped flag, the SuperTeams() rosters — is identical, so this port mirrors basic_grouping_page
// and adds the grid layout via set_items_layout.
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
//   - the GridItemsLayout is wired through set_items_layout (Span 2, HorizontalItemSpacing 5, Vertical);
//   - is_grouped + the item/group-header/group-footer templates mirror the three XAML DataTemplates;
//   - Header/Footer are the view-level boxed strings.
//
// note: the headless backend has no real grid measure/arrange pass, so the GridItemsLayout's Span /
//       spacing have no visible geometric effect in a static headless capture — the layout object is
//       still attached (its Span/spacing values are stored on the bindable layout and would drive a
//       real backend), and the grouped structure (group names, member rows, "Total members: N"
//       footers) still surfaces exactly as in basic_grouping_page.
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
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class grid_grouping_page
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

        // The group footer template root: the shared XAML twin's <StackLayout><Label BackgroundColor=
        // "Orange" Margin="0,0,0,15" /></StackLayout> (grid_grouping.xaml:28-35). A bare `<StackLayout>`
        // (no Orientation) is what the XAML loader instantiates as maui::controls::stack_layout
        // (xaml_standard_types.cpp registers "StackLayout" -> controls::stack_layout, whose Orientation
        // bindable property defaults to Vertical — see stack_layout.hpp), so this cell uses that same
        // type rather than vertical_stack_layout, matching the twin BY CONSTRUCTION. It OWNS the bound
        // orange Label as its single child (layout::add() is non-owning, so a template cell that adds a
        // freshly-created child must own it as a member — the photo_cell / line_item_cell pattern in
        // header_footer_template_page.hpp / cv_visual_states_page.hpp).
        //
        // PORT FIX: the previous version rooted the template directly at a Label carrying BOTH
        // BackgroundColor=Orange AND Margin(0,0,0,15) — a flattened single-box template that dropped the
        // twin's StackLayout wrapper. Measured on Windows (docs/comparison/captures/windows/{maui,cpp,
        // xaml}/grid_grouping_{light,dark}.png) that flattening paints the orange fill ~10px TALLER than
        // MAUI/xaml (each footer band 29px tall at y=363-391 vs MAUI/xaml's 19px at y=363-381): with the
        // margin on the same box as the background, the 15pt bottom margin's reserved space gets painted
        // orange too instead of staying transparent. Un-flattening so the margin lands on the label
        // INSIDE an unstyled StackLayout wrapper (the margin is then external to the label's own painted
        // box) reproduces MAUI's actual box model.
        class group_footer_cell final : public maui::controls::stack_layout
        {
        public:
            group_footer_cell()
            {
                label_.set_background(std::static_pointer_cast<maui::graphics::paint>(
                    std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::orange)));
                label_.set_margin(maui::core::thickness(0, 0, 0, 15)); // twin: inner Label Margin="0,0,0,15"
                add(label_);
            }

        protected:
            // Push {Binding Count, StringFormat='Total members: {0}'} → the Label when the cell's
            // BindingContext (the team_key group key) is set by the realize path.
            void on_binding_context_changed() override
            {
                maui::controls::stack_layout::on_binding_context_changed(); // propagate to children first
                if (const auto key = binding_context<team_key>())
                {
                    label_.set_text("Total members: " + std::to_string(key->count));
                }
            }

        private:
            maui::controls::label label_;
        };

        grid_grouping_page()
        {
            page_.set_title("Grid Grouping");

            // ---- the ItemsLayout: GridItemsLayout Span="2" HorizontalItemSpacing="5"
            // Orientation="Vertical" (the one structural addition vs BasicGrouping; see header note) ----
            auto layout = std::make_shared<maui::controls::grid_items_layout>(
                2, maui::controls::items_layout_orientation::vertical);
            layout->set_horizontal_item_spacing(5);
            list_.set_items_layout(layout);

            // ---- the item template: a Label bound to Member.Name (the StackLayout wrapper has no
            // headless-visible effect, so we bind the label directly — note:) ----
            auto item_cell = maui::controls::data_template::of<maui::controls::label>();
            item_cell->set_binding<std::string, member>(maui::controls::label::text_property(),
                                                        [](const member& value) { return value.name; });
            item_cell->set_value(maui::controls::margin_property(),
                                 maui::core::thickness(5, 0, 0, 0)); // shared XAML item Label Margin="5,0,0,0"
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

            // ---- the group footer template: a StackLayout > orange Label bound to Team.Count,
            // formatted like {Binding Count, StringFormat='Total members: {0:D}'} (see group_footer_cell
            // above for why this is a composite cell rather than a data_template::of<label>()) ----
            list_.set_group_footer_template(maui::controls::data_template::of<group_footer_cell>());

            // ---- IsGrouped + the view-level header/footer strings ----
            list_.set_is_grouped(true);
            list_.set_header(maui::controls::boxed_item::of(std::string{"This is a header"}));
            list_.set_footer(maui::controls::boxed_item::of(std::string{"This is a footer."}));

            // ---- ItemsSource = new SuperTeams() ----
            list_.set_items_source(build_teams());

            // page-direct CollectionView bypasses the layout safe-area inset (+ .Never) so its
            // content would render under the notch/status-bar cutout; inset the page content below the
            // container safe area (mirrors the shared XAML ContentPage SafeAreaEdges="Container").
            page_.set_safe_area_edges(maui::core::safe_area_edges{maui::core::safe_area_regions::container});
            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // PRE-MOUNT hook (gallery_host.hpp gallery_pre_mount): register group_footer_cell's handler
        // BEFORE mount_window / the collection_view realize walk. group_footer_cell is a brand-new user
        // type (like header_footer_template_page's photo_cell / cv_visual_states_page's line_item_cell),
        // so its handler isn't self-registered; the collection_view realize path resolves a template's
        // handler via THIS app's per-app handler_registry (of<TCell>() -> create_handler by the cell's
        // type_tag) — without this the group footer supplemental would silently fail to realize. It is a
        // stack_layout subclass, so it shares stack_layout's layout_handler.
        void register_handlers(maui::hosting::maui_app& app)
        {
            maui::core::register_handler<group_footer_cell, maui::core::layout_handler>(app.handlers());
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
