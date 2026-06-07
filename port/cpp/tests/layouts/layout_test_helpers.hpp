#pragma once
// Test doubles for the layout-manager tests — the C++ equivalent of the C# NSubstitute mocks in
// LayoutTestHelpers / StackLayoutManagerTests. A mock_view records its measure/arrange calls and
// returns a configured size; a mock_stack is a configurable i_stack_layout over a child list. Both
// reuse maui::controls::view<ViewInterface> for the i_view boilerplate (and to dodge the i_view
// diamond), overriding only the bits the tests drive.

#include <cstddef>
#include <memory>
#include <vector>

#include "maui/controls/view.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/i_stack_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::layouts::testing
{
    // A child view whose Measure returns a fixed size and which records its measure/arrange calls.
    class mock_view : public maui::controls::view<maui::core::i_view>
    {
    public:
        void configure(maui::graphics::size size)
        {
            measure_result = size;
            desired_size_ = size; // DesiredSize is available pre-measure too (matching the C# mock)
        }

        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override
        {
            ++measure_count;
            last_measure_width = width_constraint;
            last_measure_height = height_constraint;
            desired_size_ = measure_result;
            return measure_result;
        }

        maui::graphics::size arrange(const maui::graphics::rect& bounds) override
        {
            ++arrange_count;
            last_arrange = bounds;
            frame_ = bounds;
            return {bounds.width, bounds.height};
        }

        [[nodiscard]] maui::core::visibility visibility() const override
        {
            return visibility_value;
        }

        maui::graphics::size measure_result;
        maui::core::visibility visibility_value = maui::core::visibility::visible;
        int measure_count = 0;
        int arrange_count = 0;
        double last_measure_width = 0;
        double last_measure_height = 0;
        maui::graphics::rect last_arrange;
    };

    // A configurable stack layout over a list of child views (non-owning; the fixture owns the views).
    class mock_stack : public maui::controls::view<maui::core::i_stack_layout>
    {
    public:
        // i_container
        [[nodiscard]] int count() const override
        {
            return static_cast<int>(children.size());
        }
        [[nodiscard]] maui::core::i_view& at(int index) const override
        {
            return *children[static_cast<std::size_t>(index)];
        }
        void add(maui::core::i_view& child) override
        {
            children.push_back(&child);
        }
        void insert(int index, maui::core::i_view& child) override
        {
            children.insert(children.begin() + index, &child);
        }
        void remove_at(int index) override
        {
            children.erase(children.begin() + index);
        }
        void clear() override
        {
            children.clear();
        }
        [[nodiscard]] int index_of(const maui::core::i_view& child) const override
        {
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                if (children[i] == &child)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // i_padding / i_stack_layout
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_value;
        }
        [[nodiscard]] double spacing() const override
        {
            return spacing_value;
        }

        // Configurable view dimensions (override view<>'s frame-derived defaults).
        [[nodiscard]] double width() const override
        {
            return width_value;
        }
        [[nodiscard]] double height() const override
        {
            return height_value;
        }
        [[nodiscard]] double minimum_width() const override
        {
            return min_width_value;
        }
        [[nodiscard]] double maximum_width() const override
        {
            return max_width_value;
        }
        [[nodiscard]] double minimum_height() const override
        {
            return min_height_value;
        }
        [[nodiscard]] double maximum_height() const override
        {
            return max_height_value;
        }

        std::vector<maui::core::i_view*> children;
        maui::core::thickness padding_value;
        double spacing_value = 0;
        double width_value = maui::core::dimension::unset;
        double height_value = maui::core::dimension::unset;
        double min_width_value = maui::core::dimension::minimum;
        double max_width_value = maui::core::dimension::maximum;
        double min_height_value = maui::core::dimension::minimum;
        double max_height_value = maui::core::dimension::maximum;
    };

    // Owns the child views + the stack, mirroring the C# BuildStack / CreateTestLayout helpers.
    class stack_fixture
    {
    public:
        mock_stack stack;

        mock_view& add_view(maui::graphics::size size)
        {
            auto view = std::make_unique<mock_view>();
            view->configure(size);
            mock_view& reference = *view;
            stack.add(reference);
            owned_.push_back(std::move(view));
            return reference;
        }

        void build_stack(int view_count, double view_width, double view_height)
        {
            for (int n = 0; n < view_count; ++n)
            {
                add_view({view_width, view_height});
            }
        }

    private:
        std::vector<std::unique_ptr<mock_view>> owned_;
    };
} // namespace maui::layouts::testing
