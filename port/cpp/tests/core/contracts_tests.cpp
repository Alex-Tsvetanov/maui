// Tests for the virtual-view contracts (i_element / i_transform / i_view / i_text_style / i_text).
// An abstract interface is exercised by a conforming mock: this confirms the full surface is
// implementable, usable polymorphically through the interface references, and that the M1-deferred
// heavy sub-objects (background/semantics/clip/shadow) are null. Inside the mock, the getters whose
// name matches their return type (visibility, flow_direction, font) must qualify the type.
#include "maui/core/i_element.hpp"
#include "maui/core/i_text.hpp"
#include "maui/core/i_view.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::font_weight;
    using maui::core::i_element_handler;
    using maui::core::i_text;
    using maui::core::i_view;
    using maui::core::layout_alignment;
    using maui::core::thickness;
    using maui::core::visibility;
    using maui::graphics::color;
    using maui::graphics::rect;
    using maui::graphics::size;

    // A minimal text-bearing view implementing the full contract surface.
    struct mock_text_view : i_view, i_text
    {
        // i_element
        [[nodiscard]] const std::shared_ptr<i_element_handler>& handler() const override
        {
            return handler_;
        }
        void set_handler(std::shared_ptr<i_element_handler> value) override
        {
            handler_ = std::move(value);
        }
        [[nodiscard]] std::shared_ptr<maui::core::i_element> parent() const override
        {
            return nullptr;
        }
        // i_transform
        [[nodiscard]] double translation_x() const override
        {
            return 0;
        }
        [[nodiscard]] double translation_y() const override
        {
            return 0;
        }
        [[nodiscard]] double scale() const override
        {
            return 1;
        }
        [[nodiscard]] double scale_x() const override
        {
            return 1;
        }
        [[nodiscard]] double scale_y() const override
        {
            return 1;
        }
        [[nodiscard]] double rotation() const override
        {
            return 0;
        }
        [[nodiscard]] double rotation_x() const override
        {
            return 0;
        }
        [[nodiscard]] double rotation_y() const override
        {
            return 0;
        }
        [[nodiscard]] double anchor_x() const override
        {
            return 0.5;
        }
        [[nodiscard]] double anchor_y() const override
        {
            return 0.5;
        }
        // i_view
        [[nodiscard]] std::string_view automation_id() const override
        {
            return "";
        }
        [[nodiscard]] maui::core::flow_direction flow_direction() const override
        {
            return maui::core::flow_direction::match_parent;
        }
        [[nodiscard]] layout_alignment horizontal_layout_alignment() const override
        {
            return layout_alignment::fill;
        }
        [[nodiscard]] layout_alignment vertical_layout_alignment() const override
        {
            return layout_alignment::fill;
        }
        [[nodiscard]] maui::core::semantics* semantics() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::graphics::i_shape* clip() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::core::i_shadow* shadow() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::graphics::paint* background() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::core::visibility visibility() const override
        {
            return maui::core::visibility::visible;
        }
        [[nodiscard]] double opacity() const override
        {
            return 1.0;
        }
        [[nodiscard]] bool is_enabled() const override
        {
            return true;
        }
        [[nodiscard]] bool is_focused() const override
        {
            return is_focused_;
        }
        void set_is_focused(bool value) override
        {
            is_focused_ = value;
        }
        [[nodiscard]] bool input_transparent() const override
        {
            return false;
        }
        [[nodiscard]] rect frame() const override
        {
            return frame_;
        }
        void set_frame(rect value) override
        {
            frame_ = value;
        }
        [[nodiscard]] double width() const override
        {
            return 100;
        }
        [[nodiscard]] double minimum_width() const override
        {
            return 0;
        }
        [[nodiscard]] double maximum_width() const override
        {
            return std::numeric_limits<double>::infinity();
        }
        [[nodiscard]] double height() const override
        {
            return 50;
        }
        [[nodiscard]] double minimum_height() const override
        {
            return 0;
        }
        [[nodiscard]] double maximum_height() const override
        {
            return std::numeric_limits<double>::infinity();
        }
        [[nodiscard]] thickness margin() const override
        {
            return margin_;
        }
        [[nodiscard]] size desired_size() const override
        {
            return {};
        }
        [[nodiscard]] int z_index() const override
        {
            return 0;
        }
        size arrange(const rect& bounds) override
        {
            frame_ = bounds;
            return {bounds.width, bounds.height};
        }
        size measure(double width_constraint, double height_constraint) override
        {
            return {std::min(width_constraint, 100.0), std::min(height_constraint, 50.0)};
        }
        void invalidate_measure() override
        {
            ++measure_invalidations_;
        }
        void invalidate_arrange() override
        {
        }
        bool focus() override
        {
            is_focused_ = true;
            return true;
        }
        void unfocus() override
        {
            is_focused_ = false;
        }
        // i_text_style / i_text
        [[nodiscard]] color text_color() const override
        {
            return text_color_;
        }
        [[nodiscard]] maui::core::font font() const override
        {
            return font_;
        }
        [[nodiscard]] double character_spacing() const override
        {
            return 0;
        }
        [[nodiscard]] std::string_view text() const override
        {
            return text_;
        }

        std::shared_ptr<i_element_handler> handler_;
        rect frame_;
        thickness margin_;
        bool is_focused_ = false;
        int measure_invalidations_ = 0;
        std::string text_ = "hello";
        maui::core::font font_ = maui::core::font::of_size("Arial", 12);
        color text_color_;
    };

    TEST(contracts, usable_through_i_view_reference)
    {
        mock_text_view view;
        i_view& iview = view;
        EXPECT_EQ(iview.visibility(), visibility::visible);
        EXPECT_EQ(iview.opacity(), 1.0);
        EXPECT_TRUE(iview.is_enabled());
        EXPECT_EQ(iview.horizontal_layout_alignment(), layout_alignment::fill);
        EXPECT_EQ(iview.width(), 100.0);
        EXPECT_EQ(iview.anchor_x(), 0.5); // inherited i_transform
        EXPECT_EQ(iview.handler(), nullptr);
        EXPECT_EQ(iview.parent(), nullptr);
    }

    TEST(contracts, settable_frame_and_focus)
    {
        mock_text_view view;
        i_view& iview = view;
        iview.set_frame(rect(0, 0, 10, 20));
        EXPECT_EQ(iview.frame(), rect(0, 0, 10, 20));

        iview.set_is_focused(true);
        EXPECT_TRUE(iview.is_focused());
        iview.unfocus();
        EXPECT_FALSE(iview.is_focused());
        EXPECT_TRUE(iview.focus());
        EXPECT_TRUE(iview.is_focused());
    }

    TEST(contracts, layout_pass)
    {
        mock_text_view view;
        i_view& iview = view;
        EXPECT_EQ(iview.measure(200, 100), size(100, 50));
        EXPECT_EQ(iview.arrange(rect(0, 0, 30, 40)), size(30, 40));
        EXPECT_EQ(iview.frame(), rect(0, 0, 30, 40)); // arrange updated the frame
    }

    TEST(contracts, deferred_heavy_subobjects_are_null)
    {
        mock_text_view view;
        i_view& iview = view;
        EXPECT_EQ(iview.background(), nullptr);
        EXPECT_EQ(iview.semantics(), nullptr);
        EXPECT_EQ(iview.clip(), nullptr);
        EXPECT_EQ(iview.shadow(), nullptr);
    }

    TEST(contracts, usable_through_i_text_reference)
    {
        mock_text_view view;
        i_text& itext = view;
        EXPECT_EQ(itext.text(), "hello");
        EXPECT_EQ(itext.font().family(), "Arial");
        EXPECT_EQ(itext.font().weight(), font_weight::regular);
        EXPECT_EQ(itext.character_spacing(), 0.0);
        EXPECT_EQ(itext.text_color(), color{});
    }
} // namespace
