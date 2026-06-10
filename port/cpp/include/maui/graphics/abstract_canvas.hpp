#pragma once
// maui::graphics::abstract_canvas  <=  Microsoft.Maui.Graphics.AbstractCanvas<TState>
//
// The save/restore stack base over canvas_state: it owns the state stack, the coordinate-transform
// tracking, the stroke-size limiting and the lazy dash-pattern flush, delegating actual rendering
// to the platform_* hooks. Ported from src/Graphics/src/Graphics/AbstractCanvas.cs. A class
// template (header-only, like every template per PROFILE §3); TState is the backend's state type.
//
// Collapses vs. C# (recorded in port/STATUS.md):
//  - ICanvasStateService<TState> — CreateNew/CreateCopy become TState's default/copy construction
//    (no state service ever did more on the Apple backends); IDisposable teardown is RAII.
//  - IStringSizeService — get_string_size stays pure virtual here; each backend answers directly.

#include <cmath>
#include <concepts>
#include <cstddef>
#include <numbers>
#include <utility>
#include <vector>

#include "maui/graphics/canvas_state.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/path_f.hpp"

namespace maui::graphics
{
    template <class TState>
        requires std::derived_from<TState, canvas_state> && std::default_initializable<TState> &&
                 std::copy_constructible<TState>
    class abstract_canvas : public i_canvas
    {
    public:
        // C# AbstractCanvas.LimitStrokeScaling { set; }.
        void set_limit_stroke_scaling(bool value)
        {
            limit_stroke_scaling_ = value;
        }

        // C# AbstractCanvas.StrokeLimit { set; }.
        void set_stroke_limit(float value)
        {
            stroke_limit_ = value;
        }

        // C# AbstractCanvas.DisplayScale (virtual auto-property, default 1).
        [[nodiscard]] float display_scale() const override
        {
            return display_scale_;
        }
        void set_display_scale(float value) override
        {
            display_scale_ = value;
        }

        // C# AbstractCanvas.StrokeSize { set; } — scales the size up to the stroke limit when
        // limiting is enabled, then stores it in the state and pushes it to the platform.
        void set_stroke_size(float value) override
        {
            float size = value;

            if (limit_stroke_scaling_)
            {
                const float scale = current_state_.scale();
                const float scaled_stroke_size = scale * value;
                if (scaled_stroke_size < stroke_limit_)
                {
                    size = stroke_limit_ / scale;
                }
            }

            current_state_.set_stroke_size(size);
            platform_set_stroke_size(size);
        }

        // C# AbstractCanvas.StrokeDashPattern { set; } — stores in the state and marks the pattern
        // dirty (flushed to the platform right before the next stroked draw). C# compares by array
        // reference; the port's value compare only skips re-flushing an IDENTICAL pattern, which is
        // platform-idempotent anyway.
        void set_stroke_dash_pattern(std::vector<float> value) override
        {
            if (value != current_state_.stroke_dash_pattern())
            {
                current_state_.set_stroke_dash_pattern(std::move(value));
                stroke_dash_pattern_dirty_ = true;
            }
        }

        // C# AbstractCanvas.StrokeDashOffset { set; } — stored only; C# does not mark the pattern
        // dirty here (quirk preserved: an offset change alone is not re-flushed).
        void set_stroke_dash_offset(float value) override
        {
            current_state_.set_stroke_dash_offset(value);
        }

        // ---- stroked draws: flush the dash pattern, then delegate to the platform ----

        void draw_line(float x1, float y1, float x2, float y2) override
        {
            ensure_stroke_pattern_set();
            platform_draw_line(x1, y1, x2, y2);
        }

        void draw_arc(float x, float y, float width, float height, float start_angle, float end_angle, bool clockwise,
                      bool closed) override
        {
            ensure_stroke_pattern_set();
            platform_draw_arc(x, y, width, height, start_angle, end_angle, clockwise, closed);
        }

        void draw_rectangle(float x, float y, float width, float height) override
        {
            ensure_stroke_pattern_set();
            platform_draw_rectangle(x, y, width, height);
        }

        // C# DrawRoundedRectangle clamps the corner radius to half the rectangle's |height|/|width|.
        void draw_rounded_rectangle(float x, float y, float width, float height, float corner_radius) override
        {
            float radius = corner_radius;

            const float half_height = std::abs(height / 2);
            if (radius > half_height)
            {
                radius = half_height;
            }

            const float half_width = std::abs(width / 2);
            if (radius > half_width)
            {
                radius = half_width;
            }

            ensure_stroke_pattern_set();
            platform_draw_rounded_rectangle(x, y, width, height, radius);
        }

        void draw_ellipse(float x, float y, float width, float height) override
        {
            ensure_stroke_pattern_set();
            platform_draw_ellipse(x, y, width, height);
        }

        void draw_path(const path_f& path) override
        {
            ensure_stroke_pattern_set();
            platform_draw_path(path);
        }

        // ---- transforms: tracked on the state, then delegated to the platform ----

        // C# AbstractCanvas.Rotate(degrees, x, y) — translate(x,y) * rotate * translate(-x,-y),
        // each PRE-multiplied onto the tracked transform.
        void rotate(float degrees, float x, float y) override
        {
            const float radians = degrees_to_radians(degrees);

            matrix3x2 transform = current_state_.transform();
            transform = matrix3x2::create_translation(x, y) * transform;
            transform = matrix3x2::create_rotation(radians) * transform;
            transform = matrix3x2::create_translation(-x, -y) * transform;
            current_state_.set_transform(transform);

            platform_rotate(degrees, radians, x, y);
        }

        void rotate(float degrees) override
        {
            const float radians = degrees_to_radians(degrees);

            matrix3x2 transform = current_state_.transform();
            transform = matrix3x2::create_rotation(radians) * transform;
            current_state_.set_transform(transform);

            platform_rotate(degrees, radians);
        }

        void scale(float sx, float sy) override
        {
            matrix3x2 transform = current_state_.transform();
            transform = matrix3x2::create_scale(sx, sy) * transform;
            current_state_.set_transform(transform);

            platform_scale(sx, sy);
        }

        void translate(float tx, float ty) override
        {
            matrix3x2 transform = current_state_.transform();
            transform = matrix3x2::create_translation(tx, ty) * transform;
            current_state_.set_transform(transform);

            platform_translate(tx, ty);
        }

        void concatenate_transform(const matrix3x2& transform) override
        {
            current_state_.set_transform(transform * current_state_.transform());

            platform_concatenate_transform(transform);
        }

        // ---- the state stack ----

        // C# AbstractCanvas.SaveState — push the current state, continue on a copy of it.
        void save_state() override
        {
            state_stack_.push_back(current_state_);
            // current_state_ stays the copy (C# pushes the original and copies into current —
            // observationally identical: both are equal values after the push).
            stroke_dash_pattern_dirty_ = true;
        }

        // C# AbstractCanvas.RestoreState — pop into current (true), or reset to a fresh state (false).
        bool restore_state() override
        {
            stroke_dash_pattern_dirty_ = true;

            if (!state_stack_.empty())
            {
                current_state_ = std::move(state_stack_.back());
                state_stack_.pop_back();
                state_restored(current_state_);
                return true;
            }

            current_state_ = TState{};
            return false;
        }

        // C# AbstractCanvas.ResetState — unwind the whole stack (state_restored per pop), then a
        // fresh state.
        void reset_state() override
        {
            while (!state_stack_.empty())
            {
                current_state_ = std::move(state_stack_.back());
                state_stack_.pop_back();
                state_restored(current_state_);
            }

            current_state_ = TState{};
        }

    protected:
        abstract_canvas() = default;

        // C# AbstractCanvas.CurrentState.
        [[nodiscard]] TState& current_state()
        {
            return current_state_;
        }
        [[nodiscard]] const TState& current_state() const
        {
            return current_state_;
        }

        // C# AbstractCanvas.LimitStrokeScalingEnabled / AssignedStrokeLimit (for the platform's own
        // dash-pattern scaling, see PlatformCanvas.PlatformSetStrokeDashPattern).
        [[nodiscard]] bool limit_stroke_scaling_enabled() const
        {
            return limit_stroke_scaling_;
        }
        [[nodiscard]] float assigned_stroke_limit() const
        {
            return stroke_limit_;
        }

        // C# AbstractCanvas.StateRestored — hook for platform canvases.
        virtual void state_restored(TState& state)
        {
            (void)state;
        }

        // ---- the platform hooks (C# protected abstract Platform*) ----
        virtual void platform_set_stroke_size(float value) = 0;
        virtual void platform_set_stroke_dash_pattern(const std::vector<float>& pattern, float stroke_dash_offset,
                                                      float stroke_size) = 0;
        virtual void platform_draw_line(float x1, float y1, float x2, float y2) = 0;
        virtual void platform_draw_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                                       bool clockwise, bool closed) = 0;
        virtual void platform_draw_rectangle(float x, float y, float width, float height) = 0;
        virtual void platform_draw_rounded_rectangle(float x, float y, float width, float height,
                                                     float corner_radius) = 0;
        virtual void platform_draw_ellipse(float x, float y, float width, float height) = 0;
        virtual void platform_draw_path(const path_f& path) = 0;
        virtual void platform_rotate(float degrees, float radians, float x, float y) = 0;
        virtual void platform_rotate(float degrees, float radians) = 0;
        virtual void platform_scale(float sx, float sy) = 0;
        virtual void platform_translate(float tx, float ty) = 0;
        virtual void platform_concatenate_transform(const matrix3x2& transform) = 0;

        // GeometryUtil.DegreesToRadians.
        [[nodiscard]] static float degrees_to_radians(float degrees)
        {
            return static_cast<float>(std::numbers::pi) * degrees / 180.0F;
        }

    private:
        // C# AbstractCanvas.EnsureStrokePatternSet.
        void ensure_stroke_pattern_set()
        {
            if (stroke_dash_pattern_dirty_)
            {
                platform_set_stroke_dash_pattern(current_state_.stroke_dash_pattern(),
                                                 current_state_.stroke_dash_offset(), current_state_.stroke_size());
                stroke_dash_pattern_dirty_ = false;
            }
        }

        std::vector<TState> state_stack_;
        TState current_state_{};
        bool limit_stroke_scaling_ = false;
        float stroke_limit_ = 1;
        bool stroke_dash_pattern_dirty_ = false;
        float display_scale_ = 1;
    };
} // namespace maui::graphics
