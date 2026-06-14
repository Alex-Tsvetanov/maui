// Tests for the window overlay + adorner (G4): the window add/remove overlay surface, the overlay's
// element/adorner list, and the invalidate->draw seam recorded through the owned graphics_view's
// platform mirror. Ported from src/Controls/tests/Core.UnitTests/WindowOverlayTests.cs
// (CreateAndRemoveOverlayWindow + CreateWindowOverlayAndElements) and WindowOverlay.cs's Draw.
//
// Backend-agnostic: the overlay renders through the same graphics_view -> graphics_view_handler ->
// platform host as a developer's GraphicsView (W2-23); headless keeps the redraw count + the replay
// seat on the platform mirror, so these assertions read the same surface on every backend.
//
// §8 teardown: the window (the overlay's parent + add/remove publisher) is declared BEFORE the
// overlays and elements, so they are torn down first.
#include <memory>
#include <vector>

#include "maui/controls/graphics_view.hpp"
#include "maui/controls/window.hpp"
#include "maui/controls/window_overlay.hpp"
#include "maui/core/graphics_view_handler.hpp"
#include "maui/core/i_adorner.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_window_overlay_element.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/recording_canvas.hpp"
#include "maui/graphics/rect_f.hpp"

#include <gtest/gtest.h>

namespace
{
    using maui::controls::window;
    using maui::controls::window_overlay;

    // A drawable overlay element that records how many times it was drawn (C# TestWindowElement).
    class test_element final : public maui::core::i_window_overlay_element
    {
    public:
        void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override
        {
            ++draw_count;
            canvas.fill_rectangle(dirty_rect.x, dirty_rect.y, dirty_rect.width, dirty_rect.height);
        }
        [[nodiscard]] bool contains(const maui::graphics::point& point) const override
        {
            return point.x >= 0 && point.y >= 0; // a simple hit region for the tap test
        }
        int draw_count = 0;
    };

    // A minimal i_adorner (C# IAdorner : IWindowOverlayElement) — proves the adorner sits in the list.
    class test_adorner final : public maui::core::i_adorner
    {
    public:
        explicit test_adorner(maui::core::i_view* view) : view_(view)
        {
        }
        void draw(maui::graphics::i_canvas& /*canvas*/, const maui::graphics::rect_f& /*dirty_rect*/) override
        {
            ++draw_count;
        }
        [[nodiscard]] bool contains(const maui::graphics::point& /*point*/) const override
        {
            return false;
        }
        [[nodiscard]] float density() const override
        {
            return 1.0F;
        }
        [[nodiscard]] maui::core::i_view* visual_view() const override
        {
            return view_;
        }
        int draw_count = 0;

    private:
        maui::core::i_view* view_;
    };

    // The graphics_view platform mirror behind the overlay's draw host (its redraw counter).
    const maui::core::graphics_view_platform* platform_of(window_overlay& overlay)
    {
        auto handler = overlay.graphics_surface().handler();
        const auto* gvh = dynamic_cast<const maui::core::graphics_view_handler*>(handler.get());
        return gvh != nullptr ? gvh->typed_platform_view() : nullptr;
    }

    // ---- the window add/remove overlay surface (C# WindowOverlayTests.CreateAndRemoveOverlayWindow) ----

    TEST(window_overlay, add_then_remove_on_the_window)
    {
        window win; // publisher first (§8)
        window_overlay overlay(&win);

        // Not processed by a window yet -> not initialized.
        EXPECT_FALSE(overlay.is_platform_view_initialized());

        // First add returns true + initializes the overlay.
        EXPECT_TRUE(win.add_overlay(overlay));
        EXPECT_TRUE(overlay.is_platform_view_initialized());
        EXPECT_FALSE(win.overlays().empty());

        // Adding the same overlay again returns false.
        EXPECT_FALSE(win.add_overlay(overlay));

        // Removing returns true + deinitializes.
        EXPECT_TRUE(win.remove_overlay(overlay));
        EXPECT_FALSE(overlay.is_platform_view_initialized());
        EXPECT_TRUE(win.overlays().empty());

        // Removing again returns false.
        EXPECT_FALSE(win.remove_overlay(overlay));

        // Re-add re-initializes.
        EXPECT_TRUE(win.add_overlay(overlay));
        EXPECT_TRUE(overlay.is_platform_view_initialized());

        win.remove_overlay(overlay); // tidy before teardown
    }

    // ---- the element list (C# WindowOverlayTests.CreateWindowOverlayAndElements) ----

    TEST(window_overlay, add_and_remove_window_elements)
    {
        window win;
        window_overlay overlay(&win);
        test_element element;
        win.add_overlay(overlay);

        EXPECT_TRUE(overlay.add_window_element(element));
        EXPECT_EQ(overlay.window_elements().size(), 1U);

        // Adding the same element again returns false.
        EXPECT_FALSE(overlay.add_window_element(element));

        // Removing returns true and empties the list.
        EXPECT_TRUE(overlay.remove_window_element(element));
        EXPECT_TRUE(overlay.window_elements().empty());

        win.remove_overlay(overlay);
    }

    TEST(window_overlay, remove_window_elements_clears_the_list)
    {
        window win;
        window_overlay overlay(&win);
        test_element a;
        test_element b;
        win.add_overlay(overlay);
        overlay.add_window_element(a);
        overlay.add_window_element(b);
        EXPECT_EQ(overlay.window_elements().size(), 2U);

        overlay.remove_window_elements();
        EXPECT_TRUE(overlay.window_elements().empty());

        win.remove_overlay(overlay);
    }

    TEST(window_overlay, adorner_sits_in_the_element_list)
    {
        window win;
        window_overlay overlay(&win);
        test_adorner adorner(nullptr);
        win.add_overlay(overlay);

        EXPECT_TRUE(overlay.add_window_element(adorner));
        ASSERT_EQ(overlay.window_elements().size(), 1U);
        // The adorner is the stored element (an i_window_overlay_element borrow).
        EXPECT_EQ(overlay.window_elements().front(), static_cast<maui::core::i_window_overlay_element*>(&adorner));

        win.remove_overlay(overlay);
    }

    // ---- the invalidate -> draw seam (recorded through the graphics_view platform mirror) ----

    TEST(window_overlay, add_overlay_initializes_the_draw_host_and_invalidates)
    {
        window win;
        window_overlay overlay(&win);

        win.add_overlay(overlay); // Initialize() attaches the graphics_view handler; then Invalidate()
        const auto* platform = platform_of(overlay);
        ASSERT_NE(platform, nullptr);
        EXPECT_GE(platform->invalidations, 1); // the add_overlay Invalidate reached the host

        win.remove_overlay(overlay);
    }

    TEST(window_overlay, adding_an_element_requests_a_redraw)
    {
        window win;
        window_overlay overlay(&win);
        test_element element;
        win.add_overlay(overlay);

        const auto* platform = platform_of(overlay);
        ASSERT_NE(platform, nullptr);
        const int before = platform->invalidations;
        overlay.add_window_element(element); // C# AddWindowElement -> Invalidate
        EXPECT_GT(platform->invalidations, before);

        win.remove_overlay(overlay);
    }

    TEST(window_overlay, invalidate_replays_every_element_draw_through_the_host)
    {
        window win;
        window_overlay overlay(&win);
        test_element a;
        test_element b;
        win.add_overlay(overlay);
        overlay.add_window_element(a);
        overlay.add_window_element(b);

        // The platform mirror's replay() is the headless drawRect twin: it draws the graphics_view's
        // drawable (the overlay self-adapter), which fans out to every element's Draw.
        const auto* platform = platform_of(overlay);
        ASSERT_NE(platform, nullptr);
        maui::graphics::recording_canvas canvas;
        platform->replay(canvas, maui::graphics::rect_f(0, 0, 100, 50));

        EXPECT_EQ(a.draw_count, 1);
        EXPECT_EQ(b.draw_count, 1);
        EXPECT_FALSE(canvas.ops().empty()); // each element recorded a fill into the canvas

        win.remove_overlay(overlay);
    }

    TEST(window_overlay, hidden_overlay_draws_nothing)
    {
        window win;
        window_overlay overlay(&win);
        test_element element;
        win.add_overlay(overlay);
        overlay.add_window_element(element);
        overlay.set_is_visible(false); // C# WindowOverlay.IsVisible = false -> Draw is a no-op

        const auto* platform = platform_of(overlay);
        ASSERT_NE(platform, nullptr);
        maui::graphics::recording_canvas canvas;
        platform->replay(canvas, maui::graphics::rect_f(0, 0, 100, 50));

        EXPECT_EQ(element.draw_count, 0); // skipped while hidden
        EXPECT_TRUE(canvas.ops().empty());

        win.remove_overlay(overlay);
    }

    // ---- the tapped drive (drawable-touch hit list; C# WindowOverlay.OnTappedInternal) ----

    TEST(window_overlay, tapped_collects_hit_elements_when_drawable_touch_handling_is_on)
    {
        window win;
        window_overlay overlay(&win);
        test_element element;
        win.add_overlay(overlay);
        overlay.add_window_element(element);
        overlay.set_enable_drawable_touch_handling(true);

        std::vector<const maui::core::i_window_overlay_element*> hit;
        auto token =
            overlay.tapped.connect([&](const maui::graphics::point& /*p*/,
                                       const std::vector<const maui::core::i_window_overlay_element*>& h) { hit = h; });

        overlay.on_tapped_internal(maui::graphics::point(5, 5)); // inside the test_element hit region
        ASSERT_EQ(hit.size(), 1U);
        EXPECT_EQ(hit.front(), static_cast<const maui::core::i_window_overlay_element*>(&element));

        overlay.tapped.disconnect(token);
        win.remove_overlay(overlay);
    }
} // namespace
