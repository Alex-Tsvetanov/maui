// maui::xaml — XAML registration for control group "shell":
//   Shell, ShellItem, FlyoutItem, TabBar, ShellSection, Tab, ShellContent
//
// Pattern mirrors register_xaml_pages.cpp (the neighbouring pages group). This TU makes a <Shell>
// tree PARSE and mint via the runtime Create path (xaml_loader::load): the loader hydrates a real
// shell(item(section(content))) tree with the content_page hanging off the ShellContent. It closes
// the deferred-Shell note at register_xaml_pages.cpp:14-18 (the "pages" group left Shell out because
// its child-sink overloads + the flyout_behavior converter were unresolved).
//
// SCOPE / DOCUMENTED INCOMPATIBILITY (feasibility PARTIAL — honest scope-down):
//   The Create path (load) works: it mints the shell root itself. But the build_page / load_into
//   HARNESS path CANNOT host a <Shell> root. load_into hydrates markup INTO a pre-created object
//   (the gallery_xaml harness passes a content_page). shell : view<i_view> and
//   content_page : view<i_content_view> are UNRELATED SIBLING types (neither derives the other), so
//   with a <Shell> root the root child-sink dynamic_cast<shell*>(content_page) returns null, the sink
//   rejects the child, and the loader raises the LOUD "cannot set the content of Shell" parse error —
//   the content_page stays empty. i.e. the harness refuses a Shell root rather than silently
//   mis-hosting it. This is pinned by the shell_root_into_content_page_stays_empty loader test.
//   Closing it needs either a shell-under-page
//   subclass or the e2e.py root_type + CPP_SHELL factory infra — both explicitly OUT OF SCOPE here.
//   Therefore NO gap_shell.xaml / manifest row / gallery page is authored: the Create path is proven
//   by inline-XAML loader tests only.
//
// Converter added here (shell-group-owned, not in register_standard_xaml_converters):
//   convert_flyout_behavior  <=  Microsoft.Maui.FlyoutBehavior enum
//     Maps: "Disabled"->disabled, "Flyout"->flyout, "Locked"->locked
//
// Ownership note (the child-sink crux): shell / shell_item / shell_section OWN their children by
// shared_ptr, but the registry child-sink signature only hands a bindable_object& child. Every
// shell-tree node publicly derives std::enable_shared_from_this<base_shell_item>
// (base_shell_item.hpp:42) and the CreateValues pass has ALREADY stored the created child as a
// shared_ptr<bindable_object> in the load graph (xaml_visitors.cpp graph().add(...)) before the apply
// pass runs the sink — so shared_from_this() is live here (no bad_weak_ptr). We recover the child's
// own shared_ptr and hand it to the owning add_item/add overload; the graph's non-owning duplicate
// then lapses, leaving the shell tree as the single owner (PROFILE §8).

#include "register_xaml_groups.hpp"
#include "register_xaml_helpers.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/flyout_behavior.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // ---- local converter for flyout_behavior --------------------------------------------------

        // Microsoft.Maui.FlyoutBehavior (FlyoutBehavior.cs). C# enum member names (case-sensitive,
        // PascalCase) map to the port's enum values.
        [[nodiscard]] maui::controls::flyout_behavior convert_flyout_behavior(std::string_view text)
        {
            using maui::controls::flyout_behavior;
            static constexpr std::array<enum_entry<flyout_behavior>, 3> names{{
                {.name = "Disabled", .value = flyout_behavior::disabled},
                {.name = "Flyout", .value = flyout_behavior::flyout},
                {.name = "Locked", .value = flyout_behavior::locked},
            }};
            return parse_enum<flyout_behavior>(text, names, "maui::controls::flyout_behavior");
        }

        // ---- registry_converter bridge (xaml_convert_error -> xaml_parse_exception) ---------------
        // Mirrors the private registry_converter in xaml_standard_types.cpp / register_xaml_pages.cpp
        // so this TU is self-contained; identical implementation, scoped to this anonymous namespace.
        template <class T> [[nodiscard]] auto registry_converter(T (*convert)(std::string_view))
        {
            return [convert](const std::string& text) -> T {
                try
                {
                    return convert(text);
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what());
                }
            };
        }

        // Recover a shell-tree node's OWN shared_ptr from the loader graph (see the ownership note at
        // the top). The created node is already held by a shared_ptr<bindable_object> in the graph, so
        // shared_from_this() is live. TChild derives base_shell_item (which derives
        // enable_shared_from_this<base_shell_item>).
        template <class TChild> [[nodiscard]] std::shared_ptr<TChild> own_handle(TChild& node)
        {
            return std::static_pointer_cast<TChild>(
                static_cast<maui::controls::base_shell_item&>(node).shared_from_this());
        }

    } // anonymous namespace

    void register_xaml_shell(xaml_type_registry& types, xaml_property_registry& properties,
                             xaml_converter_registry& converters)
    {
        namespace controls = maui::controls;

        // ---- type registrations -------------------------------------------------------------------
        // Every type is default-constructible so the CreateValues create() path works (shell(),
        // shell_item()=default, flyout_item{}, tab_bar{}, shell_section()=default, tab{},
        // shell_content()=default per their headers).
        types.register_type<controls::shell>("Shell");
        types.register_type<controls::shell_item>("ShellItem");
        types.register_type<controls::flyout_item>("FlyoutItem");
        types.register_type<controls::tab_bar>("TabBar");
        types.register_type<controls::shell_section>("ShellSection");
        types.register_type<controls::tab>("Tab");
        types.register_type<controls::shell_content>("ShellContent");

        // ---- Shell (Shell.cs; view<i_view>) -------------------------------------------------------
        //
        // Shell IS view<>-derived, so the view property surface applies. The shell nodes below
        // (shell_item / shell_section / shell_content) derive element/base_shell_item and are NOT
        // view<>-derived — register_view_properties MUST NOT be called on them (it instantiates
        // view-only descriptors).
        register_view_properties<controls::shell>(properties);
        // Shell.FlyoutBehavior (flyout_behavior enum — the local converter below) + FlyoutIsPresented
        // (bool, already converter-covered).
        properties.register_bindable_property<controls::shell>("FlyoutBehavior",
                                                               controls::shell::flyout_behavior_property());
        properties.register_bindable_property<controls::shell>("FlyoutIsPresented",
                                                               controls::shell::flyout_is_presented_property());
        // Shell.Items ([ContentProperty("Items")]): implicit children AND <Shell.Items> route through
        // this one named sink. C# accepts a ShellItem, or wraps a bare ShellSection / ShellContent
        // (implicit conversions) — the port's add_item overloads mirror that (shell.hpp:70-74). We
        // recover each child's own shared_ptr (own_handle) so the shell becomes the single owner.
        properties.register_add_child<controls::shell>(
            "Items", [](controls::shell& parent, maui::core::bindable_object& child) {
                if (auto* item = dynamic_cast<controls::shell_item*>(&child))
                {
                    parent.add_item(own_handle(*item));
                    return true;
                }
                if (auto* section = dynamic_cast<controls::shell_section*>(&child))
                {
                    parent.add_item(own_handle(*section)); // wraps in an IMPL_ shell_item (C# parity)
                    return true;
                }
                if (auto* content = dynamic_cast<controls::shell_content*>(&child))
                {
                    parent.add_item(own_handle(*content)); // wraps in IMPL_ section + item (C# parity)
                    return true;
                }
                return false;
            });

        // ---- ShellItem / FlyoutItem / TabBar (ShellItem.cs) ---------------------------------------
        // BaseShellItem.Title (string; converter already covered). ShellItem.Items
        // ([ContentProperty("Items")]): child ShellSections, or a wrapped ShellContent (shell_item.hpp
        // add overloads). flyout_item / tab_bar are alias subclasses — the registry keys on the CONCRETE
        // type_tag with no base walk, so each concrete type's surface must be registered separately.
        // item_t is deduced from a tag value so one generic lambda registers all three.
        auto register_item_surface = [&properties]<class item_t>(item_t* /*tag*/) {
            properties.register_bindable_property<item_t>("Title", controls::base_shell_item::title_property());
            properties.register_add_child<item_t>("Items", [](item_t& parent, maui::core::bindable_object& child) {
                if (auto* section = dynamic_cast<controls::shell_section*>(&child))
                {
                    parent.add(own_handle(*section));
                    return true;
                }
                if (auto* content = dynamic_cast<controls::shell_content*>(&child))
                {
                    parent.add(own_handle(*content)); // wraps in an IMPL_ section (C# parity)
                    return true;
                }
                return false;
            });
        };
        register_item_surface(static_cast<controls::shell_item*>(nullptr));
        register_item_surface(static_cast<controls::flyout_item*>(nullptr));
        register_item_surface(static_cast<controls::tab_bar*>(nullptr));

        // ---- ShellSection / Tab (ShellSection.cs) -------------------------------------------------
        // BaseShellItem.Title. ShellSection.Items ([ContentProperty("Items")]): child ShellContents.
        // tab is an alias subclass — register its concrete surface too.
        auto register_section_surface = [&properties]<class section_t>(section_t* /*tag*/) {
            properties.register_bindable_property<section_t>("Title", controls::base_shell_item::title_property());
            properties.register_add_child<section_t>("Items",
                                                     [](section_t& parent, maui::core::bindable_object& child) {
                                                         auto* content = dynamic_cast<controls::shell_content*>(&child);
                                                         if (content == nullptr)
                                                         {
                                                             return false;
                                                         }
                                                         parent.add(own_handle(*content));
                                                         return true;
                                                     });
        };
        register_section_surface(static_cast<controls::shell_section*>(nullptr));
        register_section_surface(static_cast<controls::tab*>(nullptr));

        // ---- ShellContent (ShellContent.cs) -------------------------------------------------------
        // BaseShellItem.Title. ShellContent.Content ([ContentProperty("Content")]): a content_page,
        // set NON-owningly (shell_content::content() is a caller-owned raw content_page* — the loader
        // graph keeps the page alive for the load's lifetime, so the raw-pointer seam is safe during
        // hydration, exactly like FlyoutPage's Flyout/Detail panes).
        properties.register_bindable_property<controls::shell_content>("Title",
                                                                       controls::base_shell_item::title_property());
        properties.register_add_child<controls::shell_content>(
            "Content", [](controls::shell_content& parent, maui::core::bindable_object& child) {
                auto* page = dynamic_cast<controls::content_page*>(&child);
                if (page == nullptr)
                {
                    return false;
                }
                parent.set_content(page);
                return true;
            });

        // ---- converters (shell group) -------------------------------------------------------------
        // bool (FlyoutIsPresented) is already registered in the standard converters. flyout_behavior
        // is a new enum type with no prior converter registration.
        converters.register_converter<controls::flyout_behavior>(registry_converter(&convert_flyout_behavior));
    }
} // namespace maui::xaml
