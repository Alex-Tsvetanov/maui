#pragma once
// maui::tests::view_node / describe — a generic, reusable control-tree normalizer for the
// builder-vs-XAML structure-equivalence gate (tests/ui/gallery_structure_equivalence_tests.cpp).
//
// describe() walks a page's cross-platform control tree (no handlers, no mounting) and reduces it to
// a small comparable value: a recursive view_node carrying (a) a stable human-readable TYPE name
// derived from dynamic_cast checks against the concrete controls (never mangled typeid names — an
// unknown concrete type degrades to "layout"/"view"), (b) a CONSERVATIVE set of salient properties,
// and (c) the children, discovered the same way the hosting mount walks the tree: the generic
// i_container count()/at() for layouts, plus the single-content accessors of the content hosts
// (content_page/border/scroll_view/refresh_view/content_view). collection_view is deliberately a leaf:
// its realized children are template-generated at mount time, not part of the authored tree.
//
// Property policy (kept deliberately conservative so cosmetic noise — colors, fonts, size requests —
// cannot fail the structural gate): the recorded set is FIXED per control type and recorded
// UNCONDITIONALLY, except that empty strings (text/placeholder/title) are omitted. Both sides of a
// comparison run through this same function, so "unconditional" stays symmetric; defaults compare
// equal without needing a per-property "is set" probe. Growing the prop set is a policy change —
// widen it only when the corpus proves the extra signal is stable.
//
// Header-only, backend-agnostic pure C++ (inspects only the cross-platform control API), so it is
// includable from any test target that links maui_controls.

#include "maui/controls/border.hpp"
#include "maui/controls/box_view.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/content_view.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/i_container.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"

#include <format>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace maui::tests
{
    // One normalized node of a control tree. Value-comparable (defaulted ==) so a whole-tree
    // comparison is a single EXPECT_EQ whose failure prints both trees via PrintTo below.
    struct view_node
    {
        std::string type;
        std::vector<std::pair<std::string, std::string>> props;
        std::vector<view_node> children;
        friend bool operator==(const view_node&, const view_node&) = default;
    };

    namespace detail
    {
        // std::format's shortest-round-trip double rendering ("12" not "12.000000"), identical on
        // both sides of a comparison since both run through here.
        [[nodiscard]] inline std::string format_double(double value)
        {
            return std::format("{}", value);
        }

        [[nodiscard]] inline std::string format_thickness(const maui::core::thickness& value)
        {
            return std::format("{},{},{},{}", value.left, value.top, value.right, value.bottom);
        }

        inline void add_prop(view_node& node, std::string name, std::string value)
        {
            node.props.emplace_back(std::move(name), std::move(value));
        }

        inline void add_text_prop(view_node& node, std::string name, std::string_view value)
        {
            if (!value.empty()) // empty text/placeholder/title is the "not authored" default — omit
            {
                add_prop(node, std::move(name), std::string(value));
            }
        }
    } // namespace detail

    [[nodiscard]] inline view_node describe(const maui::core::i_view& view);

    namespace detail
    {
        // The per-concrete-type naming + property capture. Ordered most-derived-first where the
        // hierarchy could otherwise mis-name (none of the current entries shadow each other, but the
        // container checks below rely on this having claimed the concrete name already).
        [[nodiscard]] inline view_node describe_self(const maui::core::i_view& view)
        {
            namespace controls = maui::controls;
            view_node node;
            if (const auto* label = dynamic_cast<const controls::label*>(&view))
            {
                node.type = "label";
                add_text_prop(node, "text", label->text());
            }
            else if (const auto* button = dynamic_cast<const controls::button*>(&view))
            {
                node.type = "button";
                add_text_prop(node, "text", button->text());
            }
            else if (const auto* entry = dynamic_cast<const controls::entry*>(&view))
            {
                node.type = "entry";
                add_text_prop(node, "text", entry->text());
                add_text_prop(node, "placeholder", entry->placeholder());
            }
            else if (const auto* editor = dynamic_cast<const controls::editor*>(&view))
            {
                node.type = "editor";
                add_text_prop(node, "text", editor->text());
            }
            else if (const auto* slider = dynamic_cast<const controls::slider*>(&view))
            {
                node.type = "slider";
                add_prop(node, "minimum", format_double(slider->minimum()));
                add_prop(node, "maximum", format_double(slider->maximum()));
                add_prop(node, "value", format_double(slider->value()));
            }
            else if (dynamic_cast<const controls::image*>(&view) != nullptr)
            {
                // Source identity/URI is deliberately NOT captured (file vs font vs stream sources
                // have no common comparable key yet); the structural fact is "an image sits here".
                node.type = "image";
            }
            else if (dynamic_cast<const controls::box_view*>(&view) != nullptr)
            {
                node.type = "box_view";
            }
            else if (const auto* check_box = dynamic_cast<const controls::check_box*>(&view))
            {
                node.type = "check_box";
                add_prop(node, "is_checked", check_box->is_checked() ? "true" : "false");
            }
            else if (const auto* toggle = dynamic_cast<const controls::toggle_switch*>(&view))
            {
                node.type = "switch";
                add_prop(node, "is_toggled", toggle->is_toggled() ? "true" : "false");
            }
            else if (dynamic_cast<const controls::collection_view*>(&view) != nullptr)
            {
                node.type = "collection_view"; // leaf by policy (children are template-realized)
            }
            else if (dynamic_cast<const controls::border*>(&view) != nullptr)
            {
                node.type = "border";
            }
            else if (dynamic_cast<const controls::scroll_view*>(&view) != nullptr)
            {
                node.type = "scroll_view";
            }
            else if (dynamic_cast<const controls::refresh_view*>(&view) != nullptr)
            {
                node.type = "refresh_view";
            }
            else if (dynamic_cast<const controls::content_view*>(&view) != nullptr)
            {
                node.type = "content_view";
            }
            else if (const auto* grid = dynamic_cast<const controls::grid*>(&view))
            {
                node.type = "grid";
                add_prop(node, "rows", std::to_string(grid->row_definitions().size()));
                add_prop(node, "columns", std::to_string(grid->column_definitions().size()));
                add_prop(node, "padding", format_thickness(grid->padding()));
            }
            else if (const auto* vstack = dynamic_cast<const controls::vertical_stack_layout*>(&view))
            {
                node.type = "vertical_stack_layout";
                add_prop(node, "spacing", format_double(vstack->spacing()));
                add_prop(node, "padding", format_thickness(vstack->padding()));
            }
            else if (const auto* hstack = dynamic_cast<const controls::horizontal_stack_layout*>(&view))
            {
                node.type = "horizontal_stack_layout";
                add_prop(node, "spacing", format_double(hstack->spacing()));
                add_prop(node, "padding", format_thickness(hstack->padding()));
            }
            else if (const auto* stack = dynamic_cast<const controls::stack_layout*>(&view))
            {
                node.type = "stack_layout";
                add_prop(node, "orientation",
                         stack->orientation() == controls::stack_orientation::horizontal ? "horizontal" : "vertical");
                add_prop(node, "spacing", format_double(stack->spacing()));
                add_prop(node, "padding", format_thickness(stack->padding()));
            }
            else if (const auto* layout = dynamic_cast<const maui::core::i_layout*>(&view))
            {
                node.type = "layout"; // a concrete layout this normalizer doesn't name yet
                add_prop(node, "padding", format_thickness(layout->padding()));
            }
            else
            {
                node.type = "view"; // unknown leaf — structural presence only
            }
            return node;
        }

        // Child discovery, mirroring how the hosting mount walks the tree: the generic
        // i_container count()/at() for layouts, then the single-content hosts.
        inline void describe_children(const maui::core::i_view& view, view_node& node)
        {
            namespace controls = maui::controls;
            if (node.type == "collection_view")
            {
                return; // leaf by policy
            }
            if (const auto* container = dynamic_cast<const maui::core::i_container*>(&view))
            {
                for (int index = 0; index < container->count(); ++index)
                {
                    node.children.push_back(describe(container->at(index)));
                }
                return;
            }
            const maui::core::i_view* content = nullptr;
            if (const auto* border = dynamic_cast<const controls::border*>(&view))
            {
                content = border->content();
            }
            else if (const auto* scroll = dynamic_cast<const controls::scroll_view*>(&view))
            {
                content = scroll->content();
            }
            else if (const auto* refresh = dynamic_cast<const controls::refresh_view*>(&view))
            {
                content = refresh->content();
            }
            else if (const auto* host = dynamic_cast<const controls::content_view*>(&view))
            {
                content = host->content();
            }
            if (content != nullptr)
            {
                node.children.push_back(describe(*content));
            }
        }
    } // namespace detail

    // Normalize one view subtree.
    [[nodiscard]] inline view_node describe(const maui::core::i_view& view)
    {
        view_node node = detail::describe_self(view);
        detail::describe_children(view, node);
        return node;
    }

    // Normalize a whole page: a "content_page" root (title + padding) over its content subtree.
    [[nodiscard]] inline view_node describe(const maui::controls::content_page& page)
    {
        view_node node;
        node.type = "content_page";
        detail::add_text_prop(node, "title", page.title());
        detail::add_prop(node, "padding", detail::format_thickness(page.padding()));
        if (const maui::core::i_view* content = page.content())
        {
            node.children.push_back(describe(*content));
        }
        return node;
    }

    namespace detail
    {
        inline void print_tree(const view_node& node, std::ostream& out, int depth)
        {
            out << '\n' << std::string(static_cast<std::size_t>(depth) * 2U, ' ') << node.type;
            for (const auto& [name, value] : node.props)
            {
                out << ' ' << name << "=\"" << value << '"';
            }
            for (const view_node& child : node.children)
            {
                print_tree(child, out, depth + 1);
            }
        }
    } // namespace detail

    // gtest ADL hook: an EXPECT_EQ failure prints both trees as indented multi-line dumps, so the
    // divergence reads as a tree diff rather than one unreadable line.
    // (PrintTo is gtest's spelling — the one identifier here that legitimately isn't snake_case.)
    inline void PrintTo(const view_node& node, std::ostream* out)
    {
        detail::print_tree(node, *out, 1);
        *out << '\n';
    }
} // namespace maui::tests
