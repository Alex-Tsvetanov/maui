// maui::xaml — XAML registration for the "tables" control group:
//   TableView, TableRoot, TableSection, and the renderable cell family
//   (TextCell, EntryCell, SwitchCell, ImageCell, ViewCell).
//
// Source of truth: the port control headers (table_view.hpp / table_root.hpp / table_section.hpp /
// table_section_base.hpp / cells/*.hpp) + the C# TableView content hierarchy
// (src/Controls/src/Core/TableView/*, src/Controls/src/Core/Cells/*).
//
// Content model (mirrors the C# ContentProperty attributes + IList content):
//   - TableView [ContentProperty(nameof(Root))]  -> named "Root" child sink (single table_root).
//   - TableRoot : TableSectionBase<TableSection>  -> UNNAMED list child sink (bare <TableSection> rows).
//   - TableSection : TableSectionBase<Cell>       -> UNNAMED list child sink (bare cell rows).
//   - ViewCell [ContentProperty("View")]          -> named "View" child sink (single element).
//
// find() does NO base-type walk (register_xaml_helpers.hpp doctrine), so inherited bindables are
// RE-registered per concrete type: Title/TextColor on BOTH table_root and table_section (from
// table_section_base); IsEnabled/Height on ALL six cell types (from cell); Text/Detail/TextColor/
// DetailColor on image_cell as well as text_cell (image_cell : text_cell). Cell.Height is a PLAIN
// field (no bindable descriptor) so it routes through register_property<TCell,double>, not a bindable.
// TableView.Intent is a plain-field enum with a typed setter -> register_property + a TableIntent
// converter.
//
// Ownership: set_root / add / set_view all take an OWNING shared_ptr, but the xaml_object_graph is the
// sole owner of the parsed node — every sink passes a NON-OWNING aliasing shared_ptr
// (std::shared_ptr<T>(std::shared_ptr<void>{}, raw)), the content_view pattern
// (register_xaml_containers_content.cpp), so graph teardown does not double-free.
//
// Cells carry NO view<> surface (Cell : Element, not a view) — register_view_properties is NOT called
// on any cell, root, or section; only TableView (a view<i_table_view>) gets it.
//
// Deliberately omitted markup surface: Cell.ContextActions (MenuItem is unregistered), TextCell/
// ImageCell Command/CommandParameter (the port command is a move_only_function, not an ICommand
// bindable — bind/code only), ImageCell.ImageSource text form (binding/code only; registered bindable
// for the binding path, no text converter — mirrors ImageButton.Source). EntryCell reuses the shared
// text_alignment converter (entry/editor/label) for its Horizontal/VerticalTextAlignment.

#include "register_xaml_helpers.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/cells/entry_cell.hpp"
#include "maui/controls/cells/image_cell.hpp"
#include "maui/controls/cells/switch_cell.hpp"
#include "maui/controls/cells/text_cell.hpp"
#include "maui/controls/cells/view_cell.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/table_intent.hpp"
#include "maui/controls/table_root.hpp"
#include "maui/controls/table_section.hpp"
#include "maui/controls/table_section_base.hpp"
#include "maui/controls/table_view.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp" // enum_entry / parse_enum / xaml_convert_error
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        namespace controls = maui::controls;

        // Bridge a xaml_converters.hpp free function into the registry's error contract (the same
        // registry_converter pattern as register_xaml_items.cpp).
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

        // convert_table_intent  <=  Microsoft.Maui.Controls.TableIntent (Enum.Parse, case-sensitive).
        // C# members: Menu / Settings / Form / Data (src/.../TableView/TableIntent.cs).
        [[nodiscard]] controls::table_intent convert_table_intent(std::string_view text)
        {
            using controls::table_intent;
            static constexpr std::array<enum_entry<table_intent>, 4> names{{
                {.name = "Menu", .value = table_intent::menu},
                {.name = "Settings", .value = table_intent::settings},
                {.name = "Form", .value = table_intent::form},
                {.name = "Data", .value = table_intent::data},
            }};
            return parse_enum<table_intent>(text, names, "maui::controls::table_intent");
        }

        // The cell base surface (Cell.IsEnabled bindable + Cell.Height plain field). find() has no base
        // walk, so EVERY concrete cell type re-registers these under its own type_tag.
        template <class TCell> void register_cell_base(xaml_property_registry& properties)
        {
            properties.register_bindable_property<TCell>("IsEnabled", controls::cell::is_enabled_property());
            // Cell.Height is a PLAIN field (no height_property() descriptor) — a typed non-bindable setter.
            properties.register_property<TCell, double>(
                "Height", [](TCell& cell, const double& value) { cell.set_height(value); });
        }
    } // namespace

    void register_xaml_tables(xaml_type_registry& types, xaml_property_registry& properties,
                              xaml_converter_registry& converters)
    {
        // ---- TableIntent converter (new; register once) ----
        converters.register_converter<controls::table_intent>(registry_converter(&convert_table_intent));

        // ---- TableView ([ContentProperty(nameof(Root))]) ----
        types.register_type<controls::table_view>("TableView");
        register_view_properties<controls::table_view>(properties);
        properties.register_bindable_property<controls::table_view>("RowHeight",
                                                                    controls::table_view::row_height_property());
        properties.register_bindable_property<controls::table_view>("HasUnevenRows",
                                                                    controls::table_view::has_uneven_rows_property());
        // Intent: plain-field enum with a typed setter (uses the convert_table_intent converter above).
        properties.register_property<controls::table_view, controls::table_intent>(
            "Intent",
            [](controls::table_view& table, const controls::table_intent& value) { table.set_intent(value); });
        // Root child sink — named "Root" so BOTH <TableView><TableRoot> (implicit content) AND
        // <TableView.Root><TableRoot> (property-element) route here. NON-OWNING aliasing ptr (graph owns).
        properties.register_add_child<controls::table_view>(
            "Root", [](controls::table_view& table, maui::core::bindable_object& child) {
                auto* root = dynamic_cast<controls::table_root*>(&child);
                if (root == nullptr)
                {
                    return false;
                }
                table.set_root(std::shared_ptr<controls::table_root>(std::shared_ptr<void>{}, root));
                return true;
            });

        // ---- TableRoot (TableSectionBase<TableSection>; no ContentProperty — the IList IS the content) ----
        types.register_type<controls::table_root>("TableRoot");
        properties.register_bindable_property<controls::table_root>("Title",
                                                                    controls::table_section_base::title_property());
        properties.register_bindable_property<controls::table_root>(
            "TextColor", controls::table_section_base::text_color_property());
        // UNNAMED list child sink: sections are bare <TableSection> element children (no <TableRoot.X>).
        properties.register_add_child<controls::table_root>(
            [](controls::table_root& root, maui::core::bindable_object& child) {
                auto* section = dynamic_cast<controls::table_section*>(&child);
                if (section == nullptr)
                {
                    return false;
                }
                root.add(std::shared_ptr<controls::table_section>(std::shared_ptr<void>{}, section));
                return true;
            });

        // ---- TableSection (TableSectionBase<Cell>; no ContentProperty — the IList IS the content) ----
        types.register_type<controls::table_section>("TableSection");
        properties.register_bindable_property<controls::table_section>("Title",
                                                                       controls::table_section_base::title_property());
        properties.register_bindable_property<controls::table_section>(
            "TextColor", controls::table_section_base::text_color_property());
        // UNNAMED list child sink: cells are bare cell element children.
        properties.register_add_child<controls::table_section>(
            [](controls::table_section& section, maui::core::bindable_object& child) {
                auto* the_cell = dynamic_cast<controls::cell*>(&child);
                if (the_cell == nullptr)
                {
                    return false;
                }
                section.add(std::shared_ptr<controls::cell>(std::shared_ptr<void>{}, the_cell));
                return true;
            });

        // ---- TextCell ----
        types.register_type<controls::text_cell>("TextCell");
        register_cell_base<controls::text_cell>(properties);
        properties.register_bindable_property<controls::text_cell>("Text", controls::text_cell::text_property());
        properties.register_bindable_property<controls::text_cell>("Detail", controls::text_cell::detail_property());
        properties.register_bindable_property<controls::text_cell>("TextColor",
                                                                   controls::text_cell::text_color_property());
        properties.register_bindable_property<controls::text_cell>("DetailColor",
                                                                   controls::text_cell::detail_color_property());

        // ---- EntryCell (Text is TwoWay; the two alignment attrs reuse the shared text_alignment converter) ----
        types.register_type<controls::entry_cell>("EntryCell");
        register_cell_base<controls::entry_cell>(properties);
        properties.register_bindable_property<controls::entry_cell>("Text", controls::entry_cell::text_property());
        properties.register_bindable_property<controls::entry_cell>("Label", controls::entry_cell::label_property());
        properties.register_bindable_property<controls::entry_cell>("Placeholder",
                                                                    controls::entry_cell::placeholder_property());
        properties.register_bindable_property<controls::entry_cell>("LabelColor",
                                                                    controls::entry_cell::label_color_property());
        properties.register_bindable_property<controls::entry_cell>(
            "HorizontalTextAlignment", controls::entry_cell::horizontal_text_alignment_property());
        properties.register_bindable_property<controls::entry_cell>(
            "VerticalTextAlignment", controls::entry_cell::vertical_text_alignment_property());

        // ---- SwitchCell ----
        types.register_type<controls::switch_cell>("SwitchCell");
        register_cell_base<controls::switch_cell>(properties);
        properties.register_bindable_property<controls::switch_cell>("On", controls::switch_cell::on_property());
        properties.register_bindable_property<controls::switch_cell>("Text", controls::switch_cell::text_property());
        properties.register_bindable_property<controls::switch_cell>("OnColor",
                                                                     controls::switch_cell::on_color_property());

        // ---- ImageCell (image_cell : text_cell — RE-register the text_cell surface under image_cell) ----
        types.register_type<controls::image_cell>("ImageCell");
        register_cell_base<controls::image_cell>(properties);
        properties.register_bindable_property<controls::image_cell>("Text", controls::text_cell::text_property());
        properties.register_bindable_property<controls::image_cell>("Detail", controls::text_cell::detail_property());
        properties.register_bindable_property<controls::image_cell>("TextColor",
                                                                    controls::text_cell::text_color_property());
        properties.register_bindable_property<controls::image_cell>("DetailColor",
                                                                    controls::text_cell::detail_color_property());
        // ImageSource is shared_ptr<i_image_source>: no text converter (binding/code only) — mirrors
        // ImageButton.Source. Registered bindable so the binding path routes through apply_setter.
        properties.register_bindable_property<controls::image_cell>("ImageSource",
                                                                    controls::image_cell::image_source_property());

        // ---- ViewCell ([ContentProperty("View")]) ----
        types.register_type<controls::view_cell>("ViewCell");
        register_cell_base<controls::view_cell>(properties);
        // Named "View" so both <ViewCell><Label/> (implicit content) and <ViewCell.View> (property
        // element) route here. NON-OWNING aliasing ptr — graph owns the element.
        properties.register_add_child<controls::view_cell>(
            "View", [](controls::view_cell& cell, maui::core::bindable_object& child) {
                auto* elem = dynamic_cast<controls::element*>(&child);
                if (elem == nullptr)
                {
                    return false;
                }
                cell.set_view(std::shared_ptr<controls::element>(std::shared_ptr<void>{}, elem));
                return true;
            });
    }
} // namespace maui::xaml
