#pragma once
// maui::ui builder — declarative factories that construct controls on the heap and return an owning
// move-only view_ref<T> (PUBLIC_API_DESIGN.md §3-C). Leaf factories (label/button) plus the owning
// containers (vstack/hstack adopt their children's ownership; page hosts a single content child). Each
// factory make_shared's the control (a stable heap address, §8); the container/page additionally take over
// ownership of every child passed in, so ONE root view_ref owns the whole subtree — no member-order dance.

#include <memory>
#include <string>
#include <utility>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/ui/view_ref.hpp"

namespace maui::ui
{
    // ---- leaf controls ----
    [[nodiscard]] inline view_ref<maui::controls::label> label(std::string text = {})
    {
        auto control = std::make_shared<maui::controls::label>();
        control->set_text(std::move(text));
        return view_ref<maui::controls::label>(std::move(control));
    }

    [[nodiscard]] inline view_ref<maui::controls::button> button(std::string text = {})
    {
        auto control = std::make_shared<maui::controls::button>();
        control->set_text(std::move(text));
        return view_ref<maui::controls::button>(std::move(control));
    }

    [[nodiscard]] inline view_ref<maui::controls::entry> entry()
    {
        return view_ref<maui::controls::entry>(std::make_shared<maui::controls::entry>());
    }

    // ---- owning containers (adopt every child's ownership) ----
    template <class... Children>
    [[nodiscard]] view_ref<maui::controls::vertical_stack_layout> vstack(Children&&... children)
    {
        view_ref<maui::controls::vertical_stack_layout> node(std::make_shared<maui::controls::vertical_stack_layout>());
        (node.adopt_child(std::forward<Children>(children)), ...);
        return node;
    }

    template <class... Children>
    [[nodiscard]] view_ref<maui::controls::horizontal_stack_layout> hstack(Children&&... children)
    {
        view_ref<maui::controls::horizontal_stack_layout> node(
            std::make_shared<maui::controls::horizontal_stack_layout>());
        (node.adopt_child(std::forward<Children>(children)), ...);
        return node;
    }

    // ---- single-content page host ----
    template <class Child> [[nodiscard]] view_ref<maui::controls::content_page> page(Child&& content)
    {
        view_ref<maui::controls::content_page> node(std::make_shared<maui::controls::content_page>());
        node.adopt_content(std::forward<Child>(content));
        return node;
    }

    // ---- grid (configured with .columns(...)/.rows(...)/.cell(row, col, child) chainers on view_ref) ----
    [[nodiscard]] inline view_ref<maui::controls::grid> grid()
    {
        return view_ref<maui::controls::grid>(std::make_shared<maui::controls::grid>());
    }

    // grid_length conveniences for the .columns(...)/.rows(...) chainers.
    [[nodiscard]] inline maui::core::grid_length star()
    {
        return maui::core::grid_length::star();
    }
    [[nodiscard]] inline maui::core::grid_length automatic()
    {
        return maui::core::grid_length::automatic();
    }
    [[nodiscard]] inline maui::core::grid_length absolute(double size)
    {
        return maui::core::grid_length{size};
    }
} // namespace maui::ui
