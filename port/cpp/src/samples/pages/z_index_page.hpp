#pragma once
// maui::samples::z_index_page — ports ZIndexPage.xaml (+ ZIndexPage.xaml.cs), code-first.
//
// The MAUI page builds, in the code-behind, ten overlapping Labels inside a Grid (Root, row 1), each
// with ZIndex = n, a fixed 200x100 size, a cascading Margin (n*15) and a cycling BackgroundColor, so
// later-indexed labels paint over earlier ones. A HorizontalStackLayout (row 0) holds a readout Label
// and a Stepper; stepping the stepper rewrites Label 5's ZIndex live (and its readout text), restacking
// it among its siblings.
//
// This port mirrors that exactly with the port's z_index surface (VisualElement.ZIndex →
// view::set_z_index, which routes the change to the parent layout's "update_z_index"). The overlapping
// children live in a grid (the C# Grid Root); the stepper drives Label 5's z-index.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic: a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// Interactions demonstrated:
//   - ten overlapping labels whose paint order is set by ZIndex (later index paints on top),
//   - the stepper rewrites Label 5's ZIndex at runtime, restacking it (the C# ValueChanged handler),
//   - the readout label echoes Label 5's current z-index.
//
// The per-label Margin (the n*15 top-left cascade) is reproduced via view::set_margin (the VisualElement/
// View.Margin seam), so the ten labels step diagonally as in real .NET MAUI; the cycling BackgroundColor
// + the differing z-index make the overlap order visible. The colors mirror the C# _colors cycle
// (Aquamarine, Orange, MediumOrchid, Red, Green, Blue) via color::from_rgb.

#include <array>
#include <cstdio>
#include <memory>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stepper.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class z_index_page
    {
    public:
        z_index_page()
        {
            page_.set_title("ZIndex");

            // Root grid: row 0 (Auto) for the controls bar, row 1 (Star) for the overlapping labels.
            root_.add_row_definition(maui::core::grid_length::automatic());
            root_.add_row_definition(maui::core::grid_length::star());

            // ---- the controls bar (HorizontalStackLayout: readout + stepper), grid row 0 ----
            current_z_index_.set_text("Z-Index of Label 5: 5");
            controls_bar_.add(current_z_index_);
            controls_bar_.add(stepper_);
            root_.add(controls_bar_);
            root_.set_row(controls_bar_, 0);

            // ---- ten overlapping labels, grid row 1 ----
            for (int n = 0; n < 10; ++n)
            {
                auto label = std::make_shared<maui::controls::label>();
                char text[48];
                std::snprintf(text, sizeof(text), "This is Label %d, z-index %d", n, n);
                label->set_text(text);
                label->set_z_index(n);          // VisualElement.ZIndex — the feature under test
                label->set_width_request(200);  // C# WidthRequest = 200
                label->set_height_request(100); // C# HeightRequest = 100
                // Anchor each label top-left in its grid cell so the fixed 200x100 box does not stretch to
                // fill the Star row — otherwise every label would cover the whole cell and the diagonal
                // cascade would be invisible.
                label->set_horizontal_layout_alignment(maui::core::layout_alignment::start);
                label->set_vertical_layout_alignment(maui::core::layout_alignment::start);
                label->set_background(std::make_shared<maui::graphics::solid_paint>(pick_color(n)));
                // C# Margin = new Thickness(n*15, n*15, 0, 0): the diagonal cascade (top-left offset only).
                label->set_margin(maui::core::thickness(n * 15, n * 15, 0, 0));

                if (n == 5)
                {
                    target_ = label; // the stepper's subject (C# `target`)
                }
                labels_.push_back(label);
                root_.add(*label);
                root_.set_row(*label, 1);
            }

            // ---- the stepper drives Label 5's z-index (C# ZIndexStepper.ValueChanged) ----
            stepper_.set_minimum(0);
            stepper_.set_maximum(20);
            stepper_.set_increment(1);
            stepper_.set_value(target_->z_index()); // C# ZIndexStepper.Value = target.ZIndex (5)
            stepper_.value_changed.connect([this](double /*old_value*/, double new_value) {
                const int z = static_cast<int>(new_value);
                target_->set_z_index(z); // restacks Label 5 among its siblings
                char readout[48];
                std::snprintf(readout, sizeof(readout), "Z-Index of Label 5: %d", z);
                current_z_index_.set_text(readout);
                char text[48];
                std::snprintf(text, sizeof(text), "This is Label 5, z-index %d", z);
                target_->set_text(text);
            });

            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the bar's children and the overlapping labels
        // first, then the bar and the grid, then the page), then re-host the trees built in the ctor.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, current_z_index_, "current_z_index_");
            gallery_attach_one(app, stepper_, "stepper_");
            gallery_attach_one(app, controls_bar_, "controls_bar_");
            for (auto& label : labels_)
            {
                gallery_attach_one(app, *label, "z_label");
            }
            gallery_attach_one(app, root_, "root_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(controls_bar_); // bar hosts the readout + stepper
            gallery_rehost_layout(root_);         // grid hosts the bar + the ten overlapping labels
            gallery_rehost_content(page_);        // page hosts the grid
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::grid& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::stepper& z_index_stepper()
        {
            return stepper_;
        }
        [[nodiscard]] maui::controls::label& current_z_index()
        {
            return current_z_index_;
        }

    private:
        // The C# _colors cycle (Colors.Aquamarine, Orange, MediumOrchid, Red, Green, Blue), 0-1 RGB.
        static maui::graphics::color pick_color(int n)
        {
            static const std::array<maui::graphics::color, 6> colors = {
                maui::graphics::color::from_rgb(127, 255, 212), // Aquamarine
                maui::graphics::color::from_rgb(255, 165, 0),   // Orange
                maui::graphics::color::from_rgb(186, 85, 211),  // MediumOrchid
                maui::graphics::color::from_rgb(255, 0, 0),     // Red
                maui::graphics::color::from_rgb(0, 128, 0),     // Green
                maui::graphics::color::from_rgb(0, 0, 255),     // Blue
            };
            return colors[static_cast<std::size_t>(n % static_cast<int>(colors.size()))];
        }

        maui::controls::content_page page_;
        maui::controls::grid root_; // the C# `Root` grid
        maui::controls::horizontal_stack_layout controls_bar_;
        maui::controls::label current_z_index_;                      // the C# `CurrentZIndex`
        maui::controls::stepper stepper_;                            // the C# `ZIndexStepper`
        std::vector<std::shared_ptr<maui::controls::label>> labels_; // the ten overlapping labels
        std::shared_ptr<maui::controls::label> target_;              // Label 5 (the C# `target`)
    };
} // namespace maui::samples
