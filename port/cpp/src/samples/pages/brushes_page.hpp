#pragma once
// maui::samples::brushes_page — ports BrushesPage.xaml (+ .xaml.cs)
//
// The MAUI BrushesPage is a TabbedPage whose tabs each demonstrate a brush family painting a view's
// Background. This code-first port flattens those tab sections into one scrollable vertical stack of
// headlined "swatch" rows (a box_view per swatch, recolored via set_background_brush) so the whole
// brush surface is visible on a single headless static-capture page, then keeps the .xaml.cs "Brush
// Changes" behavior — a button that randomizes a SolidColorBrush's Color and a button that mutates a
// LinearGradientBrush stop's Color — as live, deterministic interactions driving a readout label.
//
// Demonstrated (all headless-safe maui:: API, via view::set_background_brush):
//   - SolidColorBrush: a predefined named static (brush::indigo), a Color (dark_blue), a hex color
//     (#FF9988), and the property-tag form (a fresh solid_color_brush(light_steel_blue)).
//   - LinearGradientBrush: horizontal (EndPoint 1,0), vertical (EndPoint 0,1) and diagonal (default
//     EndPoint 1,1) yellow→green gradients (StartPoint defaults to 0,0).
//   - RadialGradientBrush: upper-left (Center 0,0), centered (default Center 0.5,0.5) and lower-right
//     (Center 1,1) red→dark-blue gradients (Radius defaults to 0.5).
//   - "Brush Changes" (.xaml.cs): Update Color randomizes the solid swatch's brush Color; Update
//     Colors mutates a random stop of the linear swatch's brush — both refresh a readout, mirroring
//     OnUpdateSolidColorClicked / OnUpdateLinearColorsClicked.
//
// note: the XAML "Using CSS" tab (StyleSheet linear/radial-gradient parsing) and Polygon Fill /
// BindingContext-bound GradientStop colors are XAML/CSS-loader concerns (layer 6); they are out of
// scope for this code-first page and intentionally not reproduced.
//
// Self-contained (the value_controls_page / shapes_page pattern): the page OWNS its whole element
// tree, exposes page() and attach_handlers(maui_app).

#include <array>
#include <cstdio>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/brushes/linear_gradient_brush.hpp"
#include "maui/controls/brushes/radial_gradient_brush.hpp"
#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/point.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class brushes_page
    {
    public:
        brushes_page()
        {
            page_.set_title("Brushes");
            stack_.set_spacing(8);

            // ---------------- SolidColorBrush ----------------
            solid_headline_.set_text("SolidColorBrush");

            // Predefined Brush (the named static Brush.Indigo — an immutable_brush; copied into a fresh
            // solid_color_brush so the background owns a mutable instance).
            solid_predefined_swatch_.set_height_request(48);
            solid_predefined_swatch_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::controls::brush::indigo().color().value()));

            // SolidColorBrush (Color).
            solid_color_swatch_.set_height_request(48);
            solid_color_swatch_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::dark_blue));

            // SolidColorBrush (Hex #FF9988).
            solid_hex_swatch_.set_height_request(48);
            solid_hex_swatch_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::color::from_uint(0xFFFF9988U)));

            // SolidColorBrush (property-tag form: <SolidColorBrush Color="LightSteelBlue" />).
            solid_tag_swatch_.set_height_request(48);
            solid_tag_swatch_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::light_steel_blue));

            // ---------------- LinearGradientBrush ----------------
            linear_headline_.set_text("LinearGradientBrush (Horizontal / Vertical / Diagonal)");

            // Horizontal: StartPoint (0,0) → EndPoint (1,0), yellow@0.1 → green@1.0.
            linear_horizontal_swatch_.set_height_request(48);
            linear_horizontal_swatch_.set_background_brush(make_yellow_green_linear({1, 0}));

            // Vertical: EndPoint (0,1).
            linear_vertical_swatch_.set_height_request(48);
            linear_vertical_swatch_.set_background_brush(make_yellow_green_linear({0, 1}));

            // Diagonal: default EndPoint (1,1).
            linear_diagonal_swatch_.set_height_request(48);
            linear_diagonal_swatch_.set_background_brush(make_yellow_green_linear({1, 1}));

            // ---------------- RadialGradientBrush ----------------
            radial_headline_.set_text("RadialGradientBrush (Upper-left / Center / Lower-right)");

            // Upper-left: Center (0,0), Radius default 0.5, red@0.1 → dark-blue@1.0.
            radial_upper_left_swatch_.set_height_request(48);
            radial_upper_left_swatch_.set_background_brush(make_red_navy_radial({0, 0}));

            // Center: default Center (0.5,0.5).
            radial_center_swatch_.set_height_request(48);
            radial_center_swatch_.set_background_brush(make_red_navy_radial({0.5, 0.5}));

            // Lower-right: Center (1,1).
            radial_lower_right_swatch_.set_height_request(48);
            radial_lower_right_swatch_.set_background_brush(make_red_navy_radial({1, 1}));

            // ---------------- Brush Changes (the .xaml.cs interactions) ----------------
            changes_headline_.set_text("Brush Changes");
            readout_.set_text("Tap a button to mutate a brush");

            // SolidColorBrush swatch + "Update Color" (OnUpdateSolidColorClicked: randomize the Color).
            solid_mutable_brush_ = std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::red);
            solid_mutable_swatch_.set_height_request(48);
            solid_mutable_swatch_.set_background_brush(solid_mutable_brush_);

            update_solid_button_.set_text("Update Color");
            update_solid_button_.clicked.connect([this] { on_update_solid_color(); });

            // LinearGradientBrush swatch + "Update Colors" (OnUpdateLinearColorsClicked: mutate a random
            // stop's Color). Keep the brush + its two stops so a click can recolor one of them.
            linear_mutable_stops_.push_back(
                std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::red, 0.0F));
            linear_mutable_stops_.push_back(
                std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::orange, 1.0F));
            linear_mutable_brush_ = std::make_shared<maui::controls::linear_gradient_brush>(
                linear_mutable_stops_, maui::graphics::point{0, 0}, maui::graphics::point{1, 1});
            linear_mutable_swatch_.set_height_request(48);
            linear_mutable_swatch_.set_background_brush(linear_mutable_brush_);

            update_linear_button_.set_text("Update Colors");
            update_linear_button_.clicked.connect([this] { on_update_linear_colors(); });

            buttons_row_.set_spacing(8);
            buttons_row_.add(update_solid_button_);
            buttons_row_.add(update_linear_button_);

            // ---------------- assemble ----------------
            stack_.add(solid_headline_);
            stack_.add(solid_predefined_swatch_);
            stack_.add(solid_color_swatch_);
            stack_.add(solid_hex_swatch_);
            stack_.add(solid_tag_swatch_);
            stack_.add(linear_headline_);
            stack_.add(linear_horizontal_swatch_);
            stack_.add(linear_vertical_swatch_);
            stack_.add(linear_diagonal_swatch_);
            stack_.add(radial_headline_);
            stack_.add(radial_upper_left_swatch_);
            stack_.add(radial_center_swatch_);
            stack_.add(radial_lower_right_swatch_);
            stack_.add(changes_headline_);
            stack_.add(readout_);
            stack_.add(solid_mutable_swatch_);
            stack_.add(linear_mutable_swatch_);
            stack_.add(buttons_row_);

            scroller_.set_content(stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the stack's children, the buttons-row children,
        // the buttons row, the stack, the scroll_view, then the page), then re-host the ctor-built tree.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& view, const char* name) { gallery_attach_one(app, view, name); };

            one(solid_headline_, "solid_headline_");
            one(solid_predefined_swatch_, "solid_predefined_swatch_");
            one(solid_color_swatch_, "solid_color_swatch_");
            one(solid_hex_swatch_, "solid_hex_swatch_");
            one(solid_tag_swatch_, "solid_tag_swatch_");
            one(linear_headline_, "linear_headline_");
            one(linear_horizontal_swatch_, "linear_horizontal_swatch_");
            one(linear_vertical_swatch_, "linear_vertical_swatch_");
            one(linear_diagonal_swatch_, "linear_diagonal_swatch_");
            one(radial_headline_, "radial_headline_");
            one(radial_upper_left_swatch_, "radial_upper_left_swatch_");
            one(radial_center_swatch_, "radial_center_swatch_");
            one(radial_lower_right_swatch_, "radial_lower_right_swatch_");
            one(changes_headline_, "changes_headline_");
            one(readout_, "readout_");
            one(solid_mutable_swatch_, "solid_mutable_swatch_");
            one(linear_mutable_swatch_, "linear_mutable_swatch_");
            one(update_solid_button_, "update_solid_button_");
            one(update_linear_button_, "update_linear_button_");
            one(buttons_row_, "buttons_row_");
            one(stack_, "stack_");
            one(scroller_, "scroller_");
            one(page_, "page_");

            gallery_rehost_layout(buttons_row_);
            gallery_rehost_layout(stack_);
            gallery_rehost_content(scroller_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::box_view& solid_mutable_swatch()
        {
            return solid_mutable_swatch_;
        }
        [[nodiscard]] maui::controls::box_view& linear_mutable_swatch()
        {
            return linear_mutable_swatch_;
        }
        [[nodiscard]] maui::controls::button& update_solid_button()
        {
            return update_solid_button_;
        }
        [[nodiscard]] maui::controls::button& update_linear_button()
        {
            return update_linear_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        // A yellow@0.1 → green@1.0 LinearGradientBrush from (0,0) to end_point (the XAML swatch recipe).
        static std::shared_ptr<maui::controls::linear_gradient_brush> make_yellow_green_linear(
            maui::graphics::point end_point)
        {
            std::vector<std::shared_ptr<maui::controls::gradient_stop>> stops;
            stops.push_back(std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::yellow, 0.1F));
            stops.push_back(std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::green, 1.0F));
            return std::make_shared<maui::controls::linear_gradient_brush>(std::move(stops),
                                                                           maui::graphics::point{0, 0}, end_point);
        }

        // A red@0.1 → dark-blue@1.0 RadialGradientBrush centered at `center`, Radius default 0.5.
        static std::shared_ptr<maui::controls::radial_gradient_brush> make_red_navy_radial(maui::graphics::point center)
        {
            std::vector<std::shared_ptr<maui::controls::gradient_stop>> stops;
            stops.push_back(std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::red, 0.1F));
            stops.push_back(std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::dark_blue, 1.0F));
            return std::make_shared<maui::controls::radial_gradient_brush>(std::move(stops), center, 0.5);
        }

        // OnUpdateSolidColorClicked: pull the next color from the deterministic palette and set it on the
        // solid swatch's brush (the .xaml.cs randomizes; the port cycles a fixed palette so the static
        // capture is reproducible).
        void on_update_solid_color()
        {
            const maui::graphics::color color = next_color();
            solid_mutable_brush_->set_color(color);
            char text[64];
            std::snprintf(text, sizeof(text), "SolidColorBrush color #%08X", color.to_uint());
            readout_.set_text(text);
        }

        // OnUpdateLinearColorsClicked: recolor the next stop (cycling 0/1) of the linear swatch's brush.
        void on_update_linear_colors()
        {
            const std::size_t index = stop_cursor_ % linear_mutable_stops_.size();
            ++stop_cursor_;
            const maui::graphics::color color = next_color();
            linear_mutable_stops_[index]->set_color(color);
            char text[80];
            std::snprintf(text, sizeof(text), "LinearGradientBrush stop %zu -> #%08X", index, color.to_uint());
            readout_.set_text(text);
        }

        // The deterministic stand-in for GetRandomColor() (the static capture must be reproducible).
        maui::graphics::color next_color()
        {
            static const std::array<maui::graphics::color, 6> palette{
                maui::graphics::colors::crimson, maui::graphics::colors::teal,      maui::graphics::colors::gold,
                maui::graphics::colors::purple,  maui::graphics::colors::sea_green, maui::graphics::colors::hot_pink};
            const maui::graphics::color color = palette.at(color_cursor_ % palette.size());
            ++color_cursor_;
            return color;
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label solid_headline_;
        maui::controls::box_view solid_predefined_swatch_;
        maui::controls::box_view solid_color_swatch_;
        maui::controls::box_view solid_hex_swatch_;
        maui::controls::box_view solid_tag_swatch_;

        maui::controls::label linear_headline_;
        maui::controls::box_view linear_horizontal_swatch_;
        maui::controls::box_view linear_vertical_swatch_;
        maui::controls::box_view linear_diagonal_swatch_;

        maui::controls::label radial_headline_;
        maui::controls::box_view radial_upper_left_swatch_;
        maui::controls::box_view radial_center_swatch_;
        maui::controls::box_view radial_lower_right_swatch_;

        maui::controls::label changes_headline_;
        maui::controls::label readout_;
        maui::controls::box_view solid_mutable_swatch_;
        maui::controls::box_view linear_mutable_swatch_;
        maui::controls::horizontal_stack_layout buttons_row_;
        maui::controls::button update_solid_button_;
        maui::controls::button update_linear_button_;

        // The brushes a button mutates (kept alive so the click handlers can recolor them).
        std::shared_ptr<maui::controls::solid_color_brush> solid_mutable_brush_;
        std::shared_ptr<maui::controls::linear_gradient_brush> linear_mutable_brush_;
        std::vector<std::shared_ptr<maui::controls::gradient_stop>> linear_mutable_stops_;

        std::size_t color_cursor_ = 0;
        std::size_t stop_cursor_ = 0;
    };
} // namespace maui::samples
