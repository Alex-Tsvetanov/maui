#pragma once
// maui::layouts::flex::item  <=  Microsoft.Maui.Layouts.Flex.Item  (the vendored Yoga-like engine)
//
// Internal flex algorithm — a faithful port of src/Core/src/Layouts/Flex.cs (the native flex engine by
// Laurent Sansonetti, .NET port by Stephane Delcroix). Lives under src/layouts/detail/ (PROFILE §3:
// internal-only helper, never in the public include/ tree). The public FlexLayout enums (flex_enums.hpp)
// cast value-for-value onto the algorithm enums below, exactly as the C# `(Flex.X)(FlexX)` casts do.
//
// item is a tree node owning a flex frame (x,y,w,h) + flexbox properties + an ordered list of children.
// The layout entry point is item::layout(in_measure_mode), called on a root item with concrete Width/
// Height. A self_sizing callback lets the host (flex_layout_manager) measure each child on demand.

#include <array>
#include <cstddef>
#include <functional>
#include <limits>
#include <vector>

namespace maui::layouts::flex
{
    enum class align_content
    {
        stretch = 1,
        center = 2,
        start = 3,
        end = 4,
        space_between = 5,
        space_around = 6,
        space_evenly = 7,
    };

    enum class align_items
    {
        stretch = 1,
        center = 2,
        start = 3,
        end = 4,
    };

    enum class align_self
    {
        auto_ = 0,
        stretch = 1,
        center = 2,
        start = 3,
        end = 4,
    };

    enum class direction
    {
        row = 0,
        row_reverse = 1,
        column = 2,
        column_reverse = 3,
    };

    enum class justify
    {
        center = 2,
        start = 3,
        end = 4,
        space_between = 5,
        space_around = 6,
        space_evenly = 7,
    };

    enum class position
    {
        relative = 0,
        absolute = 1,
    };

    enum class wrap
    {
        no_wrap = 0,
        wrap = 1,
        wrap_reverse = 2,
    };

    // Flex.Basis — the internal basis value (auto / absolute / relative). The public flex_basis converts
    // to this via flex_layout::to_basis.
    class basis
    {
    public:
        constexpr basis() = default;
        constexpr basis(float length, bool is_relative) : length_(length), is_relative_(is_relative)
        {
            is_length_ = !is_relative;
        }

        [[nodiscard]] constexpr bool is_relative() const
        {
            return is_relative_;
        }
        [[nodiscard]] constexpr bool is_auto() const
        {
            return !is_length_ && !is_relative_;
        }
        [[nodiscard]] constexpr float length() const
        {
            return length_;
        }

        static const basis auto_value;

    private:
        float length_ = 0;
        bool is_length_ = false;
        bool is_relative_ = false;
    };

    inline const basis basis::auto_value{};

    class item
    {
    public:
        // Frame: [x, y, width, height].
        std::array<float, 4> frame{};

        item() = default;
        item(float width, float height) : width_(width), height_(height)
        {
        }

        // ---- container-axis layout properties (set on the root / a nesting item) ----
        flex::align_content align_content = flex::align_content::stretch;
        flex::align_items align_items = flex::align_items::stretch;
        flex::direction direction = flex::direction::column;
        flex::justify justify_content = flex::justify::start;
        flex::wrap wrap = flex::wrap::no_wrap;
        std::array<float, 4> padding{}; // left, top, right, bottom

        // ---- per-child layout properties ----
        flex::align_self align_self = flex::align_self::auto_;
        flex::basis basis = flex::basis::auto_value;
        flex::position position = flex::position::relative;
        float grow = 0;
        float shrink = 1.0F;
        bool is_visible = true;

        // Margins (left, top, right, bottom).
        float margin_left = 0;
        float margin_top = 0;
        float margin_right = 0;
        float margin_bottom = 0;

        // Absolute-position edges (NaN = unset).
        float left = std::numeric_limits<float>::quiet_NaN();
        float top = std::numeric_limits<float>::quiet_NaN();
        float right = std::numeric_limits<float>::quiet_NaN();
        float bottom = std::numeric_limits<float>::quiet_NaN();

        // SelfSizing(item&, width, height, in_measure_mode): the host measures the child and writes back
        // the chosen w/h (NaN to leave the engine's value). Mirrors the C# SelfSizingDelegate.
        using self_sizing_delegate = std::function<void(item&, float&, float&, bool)>;
        self_sizing_delegate self_sizing;

        [[nodiscard]] float width() const
        {
            return width_;
        }
        void set_width(float value)
        {
            width_ = value;
        }
        [[nodiscard]] float height() const
        {
            return height_;
        }
        void set_height(float value)
        {
            height_ = value;
        }

        [[nodiscard]] int order() const
        {
            return order_;
        }
        void set_order(int value);

        [[nodiscard]] item* parent() const
        {
            return parent_;
        }
        [[nodiscard]] std::size_t count() const
        {
            return children_.size();
        }
        [[nodiscard]] item& child_at(std::size_t index) const
        {
            return *children_[index];
        }

        // Add / insert / remove a NON-owning child (the FlexLayout owns child Flex.Items elsewhere; here
        // we mirror C# Item : List<Item> with raw back-pointers). The added child must not already have a
        // parent.
        void add(item& child);
        void insert_at(std::size_t index, item& child);
        void remove(item& child);

        // C# Item.Layout(inMeasureMode): must be called on a root item with concrete Width/Height.
        void layout(bool in_measure_mode);

        // C# Item.MarginThickness — total margin on one axis. Public so the free-function engine helpers
        // in flex.cpp can read it (the whole header is an internal detail; there is no public-API risk).
        [[nodiscard]] float margin_thickness(bool vertical) const;

        // C# Item.ShouldOrderChildren — set when a child is added with (or has its Order set to) a non-zero
        // Order, so the engine knows to sort. Read by flex_layout::init.
        [[nodiscard]] bool should_order_children() const
        {
            return should_order_children_;
        }

    private:
        std::vector<item*> children_; // non-owning, mirrors C# List<Item>
        item* parent_ = nullptr;
        int order_ = 0;
        bool should_order_children_ = false;
        float width_ = std::numeric_limits<float>::quiet_NaN();
        float height_ = std::numeric_limits<float>::quiet_NaN();
    };

    // C# Item.layout_item — the recursive engine entry for one subtree. A free function in the (internal)
    // namespace so the anonymous-namespace layout_items helper in flex.cpp can call it; item::layout(...)
    // delegates here.
    void layout_item(item& it, float width, float height, bool in_measure_mode);
} // namespace maui::layouts::flex
