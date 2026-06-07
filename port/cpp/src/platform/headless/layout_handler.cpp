// layout_handler — headless platform recipe. The "native panel" is a child-count mirror in
// layout_platform so tests can observe that the panel tracks the control's children as they are
// added/removed/cleared. The Apple twin (real NSView subviews) is src/platform/apple/layout_handler.mm.

#include "maui/core/layout_handler.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

#include "maui/core/i_view.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    layout_platform::~layout_platform() = default;

    std::unique_ptr<layout_platform> layout_handler::create_platform_view()
    {
        return std::make_unique<layout_platform>();
    }

    void layout_handler::add(i_view& child)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->children.push_back(&child);
        }
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

    void layout_handler::insert(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        auto& children = platform->children;
        const auto position = std::min(static_cast<std::size_t>(std::max(index, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
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

    void layout_handler::update_z_index(i_view& /*child*/)
    {
        // Re-order only (no count change). The headless mirror keeps the logical child order the control
        // maintains; honoring z_index ordering is deferred (the M3 managers do not yet read z_index).
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
