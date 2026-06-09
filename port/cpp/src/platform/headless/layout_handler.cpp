// layout_handler — headless platform recipe. The "native panel" is a child-count mirror in
// layout_platform so tests can observe that the panel tracks the control's children as they are
// added/removed/cleared. The Apple twin (real NSView subviews) is src/platform/apple/layout_handler.mm.

#include "maui/core/layout_handler.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

#include "maui/core/i_view.hpp"
#include "maui/core/layout_z_order.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    // Insert `child` into the subview mirror at `index` (clamped to [0, size]) — the headless analog of
    // InsertSubview. A negative index (e.g. an unfound view) appends, matching the Apple fallback.
    void insert_at(std::vector<maui::core::i_view*>& children, int index, maui::core::i_view& child)
    {
        const auto position = std::min(static_cast<std::size_t>(std::max(index, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
    }
} // namespace

namespace maui::core
{
    layout_platform::~layout_platform() = default;

    // Headless: record the ClipsToBounds mirror (the Apple twin pushes layer.masksToBounds).
    void layout_platform::update_clips_to_bounds(bool value)
    {
        clips_to_bounds = value;
    }

    std::unique_ptr<layout_platform> layout_handler::create_platform_view()
    {
        return std::make_unique<layout_platform>();
    }

    // C# LayoutHandler.Add inserts at GetLayoutHandlerIndex (the child's z-ordered position), not the end —
    // so the subview mirror stays front-to-back by z-index. The child is already in the layout's logical
    // list when this runs (the control appends before invoking "add").
    void layout_handler::add(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child)
                                                     : static_cast<int>(platform->children.size());
        insert_at(platform->children, target, child);
    }

    void layout_handler::remove(i_view& child)
    {
        if (auto* platform = typed_platform_view())
        {
            std::erase(platform->children, &child);
        }
    }

    void layout_handler::clear()
    {
        if (auto* platform = typed_platform_view())
        {
            platform->children.clear();
        }
    }

    // C# LayoutHandler.Insert also places the subview at GetLayoutHandlerIndex (the z-ordered position),
    // not the logical `index` — the panel's subview order is z-index-driven, so the logical insert position
    // is irrelevant to the native stacking. The child is already in the logical list when this runs.
    void layout_handler::insert(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : index;
        insert_at(platform->children, target, child);
    }

    void layout_handler::update(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        auto& children = platform->children;
        if (index >= 0 && static_cast<std::size_t>(index) < children.size())
        {
            children[static_cast<std::size_t>(index)] = &child; // replace-in-place: count is unchanged
        }
    }

    // C# LayoutHandler.EnsureZIndexOrder: move `child`'s subview to its z-ordered position. Re-order only
    // (no count change). The headless mirror reproduces it on the child list so the z-order is observable.
    void layout_handler::update_z_index(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        auto& children = platform->children;
        const auto current = std::ranges::find(children, &child);
        if (current == children.end())
        {
            return; // not hosted (currentIndex == -1)
        }
        const int target = get_layout_handler_index(*virtual_view(), child);
        if (target < 0)
        {
            return;
        }
        children.erase(current);
        insert_at(children, target, child);
    }

    maui::graphics::size layout_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // A layout computes its own size through its layout_manager (the control overrides measure to
        // delegate to the manager, not the handler), so the handler reports nothing here.
        return {0, 0};
    }

    void layout_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native panel to position; children are arranged by the layout_manager directly.
    }
} // namespace maui::core
