// flex — the vendored flexbox engine. A faithful port of src/Core/src/Layouts/Flex.cs. See flex.hpp.

#include "flex.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace maui::layouts::flex
{
    namespace
    {
        // C# absolute_size: explicit size, else (pos1 & pos2 set ? dim - pos2 - pos1 : 0).
        float absolute_size(float val, float pos1, float pos2, float dim)
        {
            if (!std::isnan(val))
            {
                return val;
            }
            return (!std::isnan(pos1) && !std::isnan(pos2)) ? dim - pos2 - pos1 : 0.0F;
        }

        // C# absolute_pos: pos1 if set, else (pos2 set ? dim - size - pos2 : 0).
        float absolute_pos(float pos1, float pos2, float size, float dim)
        {
            if (!std::isnan(pos1))
            {
                return pos1;
            }
            return !std::isnan(pos2) ? dim - size - pos2 : 0.0F;
        }

        align_items child_align(const item& child, const item& parent)
        {
            return child.align_self == align_self::auto_ ? parent.align_items
                                                         : static_cast<align_items>(child.align_self);
        }

        // C# layout_align(Justify, ...) — main-axis distribution; writes pos + spacing.
        void layout_align_main(justify align, float flex_dim, int children_count, float& pos_p, float& spacing_p)
        {
            if (flex_dim < 0)
            {
                throw std::invalid_argument("flex_dim must not be negative");
            }
            pos_p = 0;
            spacing_p = 0;

            switch (align)
            {
                case justify::start:
                    return;
                case justify::end:
                    pos_p = flex_dim;
                    return;
                case justify::center:
                    pos_p = flex_dim / 2;
                    return;
                case justify::space_between:
                    if (children_count > 0)
                    {
                        spacing_p = flex_dim / static_cast<float>(children_count - 1);
                    }
                    return;
                case justify::space_around:
                    if (children_count > 0)
                    {
                        spacing_p = flex_dim / static_cast<float>(children_count);
                        pos_p = spacing_p / 2;
                    }
                    return;
                case justify::space_evenly:
                    if (children_count > 0)
                    {
                        spacing_p = flex_dim / static_cast<float>(children_count + 1);
                        pos_p = spacing_p;
                    }
                    return;
            }
            throw std::invalid_argument("justify option not handled");
        }

        // C# layout_align(AlignContent, ...) — cross-axis line distribution; writes pos + spacing.
        void layout_align_content(align_content align, float flex_dim, std::size_t children_count, float& pos_p,
                                  float& spacing_p)
        {
            if (flex_dim < 0)
            {
                throw std::invalid_argument("flex_dim must not be negative");
            }
            pos_p = 0;
            spacing_p = 0;

            switch (align)
            {
                case align_content::start:
                    return;
                case align_content::end:
                    pos_p = flex_dim;
                    return;
                case align_content::center:
                    pos_p = flex_dim / 2;
                    return;
                case align_content::space_between:
                    if (children_count > 0)
                    {
                        spacing_p = flex_dim / static_cast<float>(children_count - 1);
                    }
                    return;
                case align_content::space_around:
                    if (children_count > 0)
                    {
                        spacing_p = flex_dim / static_cast<float>(children_count);
                        pos_p = spacing_p / 2;
                    }
                    return;
                case align_content::space_evenly:
                    if (children_count > 0)
                    {
                        spacing_p = flex_dim / static_cast<float>(children_count + 1);
                        pos_p = spacing_p;
                    }
                    return;
                case align_content::stretch:
                    spacing_p = flex_dim / static_cast<float>(children_count);
                    return;
            }
            throw std::invalid_argument("align_content option not handled");
        }

        // One computed line of a wrapping layout (C# flex_layout.flex_layout_line).
        struct flex_layout_line
        {
            int child_begin = 0;
            int child_end = 0;
            float size = 0;
        };

        // The per-pass working state (C# struct flex_layout). frame_*_i index into item.frame.
        struct flex_layout
        {
            bool wrap = false;
            bool reverse = false;  // main axis reversed
            bool reverse2 = false; // cross axis reversed (wrap only)
            bool vertical = false;
            float size_dim = 0;  // main axis parent size
            float align_dim = 0; // cross axis parent size
            std::size_t frame_pos_i = 0;
            std::size_t frame_pos2_i = 0;
            std::size_t frame_size_i = 0;
            std::size_t frame_size2_i = 0;
            std::optional<std::vector<std::size_t>> ordered_indices;

            float line_dim = 0;
            float flex_dim = 0;
            float extra_flex_dim = 0;
            float flex_grows = 0;
            float flex_shrinks = 0;
            float pos2 = 0;

            bool need_lines = false;
            std::vector<flex_layout_line> lines;
            float lines_sizes = 0;

            void reset()
            {
                line_dim = wrap ? 0.0F : align_dim;
                flex_dim = size_dim;
                extra_flex_dim = 0;
                flex_grows = 0;
                flex_shrinks = 0;
            }

            void init(const item& it, float width, float height)
            {
                if (it.padding[0] < 0 || it.padding[1] < 0 || it.padding[2] < 0 || it.padding[3] < 0)
                {
                    throw std::invalid_argument("padding must not be negative");
                }

                // NOTE: this mirrors the C# source verbatim (`width - PaddingLeft + PaddingRight`). The
                // signs look asymmetric but reproducing them is required for behavioral fidelity.
                width = std::max(0.0F, width - it.padding[0] + it.padding[2]);
                height = std::max(0.0F, height - it.padding[1] + it.padding[3]);

                reverse = it.direction == direction::row_reverse || it.direction == direction::column_reverse;
                vertical = true;
                switch (it.direction)
                {
                    case direction::row:
                    case direction::row_reverse:
                        vertical = false;
                        size_dim = width;
                        align_dim = height;
                        frame_pos_i = 0;
                        frame_pos2_i = 1;
                        frame_size_i = 2;
                        frame_size2_i = 3;
                        break;
                    case direction::column:
                    case direction::column_reverse:
                        size_dim = height;
                        align_dim = width;
                        frame_pos_i = 1;
                        frame_pos2_i = 0;
                        frame_size_i = 3;
                        frame_size2_i = 2;
                        break;
                }

                ordered_indices.reset();
                if (it.should_order_children() && it.count() > 0)
                {
                    // Stable sort of indices by each child's Order (std::stable_sort preserves insertion
                    // order for equal Order, matching .NET's stable OrderBy).
                    // std::views::iota (the C++20 range factory), NOT std::iota / std::ranges::iota: the NDK
                    // r27 libc++ 18 the android backend cross-compiles against lacks the C++23 ranges::iota
                    // algorithm (libc++ 20), while plain std::iota trips modernize-use-ranges on the host
                    // toolchain. Building the index vector from the views::iota factory is a ranges form
                    // clean on BOTH toolchains (same libc++-18-gap family as the from_chars accommodation).
                    const auto sequence = std::views::iota(std::size_t{0}, it.count());
                    std::vector<std::size_t> indices(sequence.begin(), sequence.end());
                    std::ranges::stable_sort(indices, [&it](std::size_t a, std::size_t b) {
                        return it.child_at(a).order() < it.child_at(b).order();
                    });
                    ordered_indices = std::move(indices);
                }

                flex_dim = 0;
                flex_grows = 0;
                flex_shrinks = 0;

                reverse2 = false;
                wrap = it.wrap != flex::wrap::no_wrap;
                if (wrap)
                {
                    if (it.wrap == flex::wrap::wrap_reverse)
                    {
                        reverse2 = true;
                        pos2 = align_dim;
                    }
                }
                else
                {
                    pos2 = vertical ? it.padding[0] : it.padding[1];
                }

                need_lines = wrap && it.align_content != align_content::start;
                lines.clear();
                lines_sizes = 0;
            }

            [[nodiscard]] item& child_at(const item& it, std::size_t i) const
            {
                return it.child_at(ordered_indices ? (*ordered_indices)[i] : i);
            }
        };

        // C# layout_items: position a span of children along the main axis, growing/shrinking + cross-aligning.
        void layout_items(item& it, int child_begin, int child_end, int children_count, flex_layout& layout,
                          bool in_measure_mode)
        {
            if (children_count > (child_end - child_begin))
            {
                throw std::invalid_argument("children_count must not exceed the requested range");
            }
            if (children_count <= 0)
            {
                return;
            }
            if (layout.flex_dim > 0 && layout.extra_flex_dim > 0)
            {
                layout.flex_dim += layout.extra_flex_dim;
            }

            float pos = 0;
            float spacing = 0;
            if (layout.flex_grows == 0 && layout.flex_dim > 0)
            {
                layout_align_main(it.justify_content, layout.flex_dim, children_count, pos, spacing);
            }

            if (layout.reverse)
            {
                pos = layout.size_dim - pos;
            }

            if (layout.reverse)
            {
                pos -= layout.vertical ? it.padding[3] : it.padding[2];
            }
            else
            {
                pos += layout.vertical ? it.padding[1] : it.padding[0];
            }
            if (layout.wrap && layout.reverse2)
            {
                layout.pos2 -= layout.line_dim;
            }

            for (int i = child_begin; i < child_end; ++i)
            {
                item& child = layout.child_at(it, static_cast<std::size_t>(i));
                if (!child.is_visible)
                {
                    continue;
                }
                if (child.position == position::absolute)
                {
                    continue;
                }

                // Grow or shrink the main-axis size.
                float flex_size = 0;
                if (layout.flex_dim > 0)
                {
                    // Distribute only the free space proportionally (issue #34464): flex_dim was inflated
                    // by extra_flex_dim, so recover the real free space by subtracting it back.
                    const float free_space = std::max(0.0F, layout.flex_dim - layout.extra_flex_dim);
                    if (child.grow != 0)
                    {
                        flex_size = (free_space / layout.flex_grows) * child.grow;
                    }
                }
                else if (layout.flex_dim < 0)
                {
                    if (child.shrink != 0)
                    {
                        flex_size = (layout.flex_dim / layout.flex_shrinks) * child.shrink;
                    }
                }
                child.frame.at(layout.frame_size_i) += flex_size;

                // Cross-axis position (+ stretch).
                const float align_size = child.frame.at(layout.frame_size2_i);
                float align_pos = layout.pos2;
                switch (child_align(child, it))
                {
                    case align_items::end:
                        align_pos +=
                            layout.line_dim - align_size - (layout.vertical ? child.margin_right : child.margin_bottom);
                        break;
                    case align_items::center:
                        align_pos += (layout.line_dim / 2) - (align_size / 2) +
                                     ((layout.vertical ? child.margin_left : child.margin_top) -
                                      (layout.vertical ? child.margin_right : child.margin_bottom));
                        break;
                    case align_items::stretch:
                        if (align_size == 0)
                        {
                            child.frame.at(layout.frame_size2_i) =
                                layout.line_dim - ((layout.vertical ? child.margin_left : child.margin_top) +
                                                   (layout.vertical ? child.margin_right : child.margin_bottom));
                        }
                        align_pos += layout.vertical ? child.margin_left : child.margin_top;
                        break;
                    case align_items::start:
                        align_pos += layout.vertical ? child.margin_left : child.margin_top;
                        break;
                }
                child.frame.at(layout.frame_pos2_i) = align_pos;

                // Main-axis position.
                if (layout.reverse)
                {
                    pos -= layout.vertical ? child.margin_bottom : child.margin_right;
                    pos -= child.frame.at(layout.frame_size_i);
                    child.frame.at(layout.frame_pos_i) = pos;
                    pos -= spacing;
                    pos -= layout.vertical ? child.margin_top : child.margin_left;
                }
                else
                {
                    pos += layout.vertical ? child.margin_top : child.margin_left;
                    child.frame.at(layout.frame_pos_i) = pos;
                    pos += child.frame.at(layout.frame_size_i);
                    pos += spacing;
                    pos += layout.vertical ? child.margin_bottom : child.margin_right;
                }

                // Now that the child has a frame, layout its own children.
                layout_item(child, child.frame[2], child.frame[3], in_measure_mode);
            }

            if (layout.wrap && !layout.reverse2)
            {
                layout.pos2 += layout.line_dim;
            }

            if (layout.need_lines)
            {
                layout.lines.push_back({.child_begin = child_begin, .child_end = child_end, .size = layout.line_dim});
                layout.lines_sizes += layout.line_dim;
            }

            if (layout.reverse && layout.size_dim == 0)
            {
                // Reversed layout with no fixed size: flip positions across the axis by the tracked offset.
                for (int i = child_begin; i < child_end; ++i)
                {
                    item& child = layout.child_at(it, static_cast<std::size_t>(i));
                    if (!child.is_visible || child.position == position::absolute)
                    {
                        continue;
                    }
                    child.frame.at(layout.frame_pos_i) = child.frame.at(layout.frame_pos_i) - pos;
                }
            }
        }
    } // namespace

    void item::set_order(int value)
    {
        order_ = value;
        if (order_ != 0 && parent_ != nullptr)
        {
            parent_->should_order_children_ = true;
        }
    }

    void item::add(item& child)
    {
        if (this == &child)
        {
            throw std::invalid_argument("cannot add item into self");
        }
        if (child.parent_ != nullptr)
        {
            throw std::invalid_argument("child already has a parent");
        }
        children_.push_back(&child);
        child.parent_ = this;
        should_order_children_ = should_order_children_ || child.order_ != 0;
    }

    void item::insert_at(std::size_t index, item& child)
    {
        if (this == &child)
        {
            throw std::invalid_argument("cannot add item into self");
        }
        if (child.parent_ != nullptr)
        {
            throw std::invalid_argument("child already has a parent");
        }
        children_.insert(children_.begin() + static_cast<std::ptrdiff_t>(index), &child);
        child.parent_ = this;
        should_order_children_ = should_order_children_ || child.order_ != 0;
    }

    void item::remove(item& child)
    {
        const auto found = std::ranges::find(children_, &child);
        if (found != children_.end())
        {
            (*found)->parent_ = nullptr;
            children_.erase(found);
        }
    }

    float item::margin_thickness(bool vertical) const
    {
        return vertical ? margin_top + margin_bottom : margin_left + margin_right;
    }

    void item::layout(bool in_measure_mode)
    {
        if (parent_ != nullptr)
        {
            throw std::logic_error("layout() must be called on a root item");
        }
        if (std::isnan(width_) || std::isnan(height_))
        {
            throw std::logic_error("layout() requires concrete Width and Height");
        }
        if (self_sizing)
        {
            throw std::logic_error("layout() cannot be called on an item with self_sizing set");
        }
        layout_item(*this, width_, height_, in_measure_mode);
    }

    void layout_item(item& it, float width, float height, bool in_measure_mode)
    {
        if (it.count() == 0)
        {
            return;
        }

        flex_layout layout;
        layout.init(it, width, height);
        layout.reset();

        int last_layout_child = 0;
        int relative_children_count = 0;
        for (int i = 0; std::cmp_less(i, it.count()); ++i)
        {
            item& child = layout.child_at(it, static_cast<std::size_t>(i));
            if (!child.is_visible)
            {
                continue;
            }

            // Absolutely-positioned items get their frame directly and are skipped in the flex pass.
            if (child.position == position::absolute)
            {
                child.frame[2] = absolute_size(child.width(), child.left, child.right, width);
                child.frame[3] = absolute_size(child.height(), child.top, child.bottom, height);
                child.frame[0] = absolute_pos(child.left, child.right, child.frame[2], width);
                child.frame[1] = absolute_pos(child.top, child.bottom, child.frame[3], height);

                layout_item(child, child.frame[2], child.frame[3], in_measure_mode);
                continue;
            }

            child.frame[0] = 0;
            child.frame[1] = 0;
            child.frame[2] = child.width();
            child.frame[3] = child.height();

            // Main-axis size defaults to 0.
            if (std::isnan(child.frame.at(layout.frame_size_i)))
            {
                child.frame.at(layout.frame_size_i) = 0;
            }

            // Cross-axis size defaults to the parent size (or the line size in wrap mode, set later).
            if (std::isnan(child.frame.at(layout.frame_size2_i)))
            {
                if (layout.wrap)
                {
                    layout.need_lines = true;
                }
                else
                {
                    child.frame.at(layout.frame_size2_i) =
                        (layout.vertical ? width : height) - child.margin_thickness(!layout.vertical);
                }
            }

            // self_sizing callback: only non-NaN values are honored; stretch on the cross axis ignores the
            // returned cross size.
            if (child.self_sizing)
            {
                std::array<float, 2> size{child.frame[2], child.frame[3]};
                child.self_sizing(child, size[0], size[1], in_measure_mode);

                for (int j = 0; j < 2; ++j)
                {
                    const std::size_t size_off = static_cast<std::size_t>(j) + 2;
                    if (size_off == layout.frame_size2_i && child_align(child, it) == align_items::stretch &&
                        layout.align_dim > 0)
                    {
                        continue;
                    }
                    const float val = size.at(static_cast<std::size_t>(j));
                    if (!std::isnan(val))
                    {
                        child.frame.at(size_off) = val;
                    }
                }
            }

            // basis overrides the main-axis size.
            if (!child.basis.is_auto())
            {
                if (child.basis.length() < 0)
                {
                    throw std::invalid_argument("basis should be >= 0");
                }
                if (child.basis.is_relative() && child.basis.length() > 1)
                {
                    throw std::invalid_argument("relative basis should be <= 1");
                }
                float basis_len = child.basis.length();
                if (child.basis.is_relative())
                {
                    basis_len *= (layout.vertical ? height : width);
                }
                child.frame.at(layout.frame_size_i) = basis_len - child.margin_thickness(layout.vertical);
            }

            const float child_size = child.frame.at(layout.frame_size_i);
            if (layout.wrap)
            {
                if (layout.flex_dim < child_size)
                {
                    // Not enough room: lay out the items so far on this line, then start a new line.
                    layout_items(it, last_layout_child, i, relative_children_count, layout, in_measure_mode);
                    layout.reset();
                    last_layout_child = i;
                    relative_children_count = 0;
                }

                const float child_size2 = child.frame.at(layout.frame_size2_i);
                if (!std::isnan(child_size2) &&
                    child_size2 + child.margin_thickness(!layout.vertical) > layout.line_dim)
                {
                    layout.line_dim = child_size2 + child.margin_thickness(!layout.vertical);
                }
            }

            if (child.grow < 0 || child.shrink < 0)
            {
                throw std::invalid_argument("shrink and grow should be >= 0");
            }

            layout.flex_grows += child.grow;
            layout.flex_shrinks += child.shrink;

            if (layout.flex_dim > 0)
            {
                layout.flex_dim -= child_size + child.margin_thickness(layout.vertical);
            }

            ++relative_children_count;

            if (child_size > 0 && child.grow > 0)
            {
                layout.extra_flex_dim += child_size;
            }
        }

        // Lay out the remaining items (or everything in non-wrap mode).
        layout_items(it, last_layout_child, static_cast<int>(it.count()), relative_children_count, layout,
                     in_measure_mode);

        // In wrap mode, tweak each line's cross position per AlignContent + fill unset cross sizes.
        if (layout.need_lines && !layout.lines.empty())
        {
            float pos = 0;
            float spacing = 0;
            const float flex_dim = layout.align_dim - layout.lines_sizes;
            if (flex_dim > 0)
            {
                layout_align_content(it.align_content, flex_dim, layout.lines.size(), pos, spacing);
            }

            float old_pos = 0;
            if (layout.reverse2)
            {
                pos = layout.align_dim - pos;
                old_pos = layout.align_dim;
            }

            for (const auto& line : layout.lines)
            {
                for (int j = line.child_begin; j < line.child_end; ++j)
                {
                    item& child = layout.child_at(it, static_cast<std::size_t>(j));
                    if (child.position == position::absolute)
                    {
                        continue;
                    }
                    if (std::isnan(child.frame.at(layout.frame_size2_i)))
                    {
                        child.frame.at(layout.frame_size2_i) =
                            line.size + (it.align_content == align_content::stretch ? spacing : 0.0F);
                    }
                    child.frame.at(layout.frame_pos2_i) = pos + (child.frame.at(layout.frame_pos2_i) - old_pos);
                }

                if (layout.reverse2)
                {
                    pos -= line.size;
                    pos -= spacing;
                    old_pos -= line.size;
                }
                else
                {
                    pos += line.size;
                    pos += spacing;
                    old_pos += line.size;
                }
            }
        }
    }
} // namespace maui::layouts::flex
