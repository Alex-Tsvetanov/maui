#pragma once
// maui::samples::border_playground_page — ports BorderPlayground.xaml (+ BorderPlayground.xaml.cs)
//
// A self-contained, code-first interactive Border playground. It mirrors the C# gallery page
// (Pages/Core/BorderGalleries/BorderPlayground.xaml): a 2-row Grid whose top row is a single live Border
// (x:Name="BorderView") and whose bottom row is a ScrollView of controls that drive every facet of that
// border — its content, stroke shape, background brush, content background, stroke brush, stroke width,
// dash array, dash offset, line join, line cap, and per-corner radii. Dragging/typing any control re-runs
// the corresponding C# Update* method and the border re-renders.
//
// The C# code-behind seeds the four pickers' SelectedIndex (content=Label, shape=RoundRectangle,
// join=Miter, cap=Butt) and calls UpdateBackground/UpdateContentBackground/UpdateBorder/
// UpdateCornerRadius in its constructor. The shared XAML twin (port/maui-reference/pages/
// border_playground.xaml) is a STATIC markup snapshot with no code-behind, so it cannot express that
// imperative seeding — its Pickers carry only a Title (no selection) and its Entries carry only Text/
// Placeholder (no BackgroundColor tint), and MAUI's actual captured render of the twin shows that
// unseeded resting state. This port matches the TWIN's resting state (not the fully-interactive C# ctor):
// it paints the live Border once via paint_border_only() (background gradient/shape/dash — the parts
// verified to match MAUI; the STROKE color is overridden back to the twin's plain solid `Stroke="#CAC531"`
// after update_border() runs, since that call always builds a start->end gradient for the interactive C#
// ctor's sake — see paint_border_only()'s own comment) WITHOUT seeding any picker's selection or tinting
// any Entry's background. Every picker's
// Update* switch already falls through its `default:` case to the same value the C# seed would have
// picked (shape default -> round_rectangle, line join default -> miter, line cap default -> butt), so the
// Border's rendered gradient/shape/dash stays identical either way — only the pickers' own displayed
// (non-)selection and the entries' (un-tinted) backgrounds differ, matching the twin. Once wired to real
// handlers, each control's change event invokes the corresponding C# update_* method (now WITH Entry
// tinting, since that is genuine user-triggered state), which rebuilds the border's background linear-
// gradient, stroke linear-gradient, stroke shape (rectangle / round_rectangle(per-corner radii) /
// ellipse), thickness, dash array (parsed from the entry), dash offset, line join and line cap — exactly
// the C# UpdateBorder body.
//
// The page OWNS its whole element tree (the gallery_page pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# views:BasePage base (a navigation host) is reproduced as a plain content_page — the page
//         chrome BasePage adds is out of scope; the playground tree is identical.
//   note: the C# Grid RowDefinitions Height="200"/"*" lay the border over the scrollable controls;
//         reproduced as a 200-absolute row + a star row (grid_page.hpp pattern).
//   note: the C# {Binding ... StringFormat} live readout labels for each slider (Border Width / Dash Offset
//         / each Corner Radius) are element-source bindings (a deferred layer-6 XAML facility); reproduced
//         imperatively — each slider's value_changed refreshes its caption label with the live value.
//   note: the C# Color resolution Color.FromArgb(text) over the hex Entries is reproduced via
//         maui::graphics::color::from_argb (empty/invalid → transparent, matching GetColorFromString).
//   note: the content "Image" option's Source="oasis.jpg" is wired as a file_image_source("oasis.jpg")
//         (the bundled asset is host-provided); with no asset present the image is simply blank — the same
//         best-effort the C# sample shows on a missing resource (never invented).
//   note: the C# DoubleCollectionConverter on the dash-array Entry is reproduced by parsing the comma/space
//         separated doubles directly into the border's stroke_dash_array (the documented vector form;
//         border.hpp records the DoubleCollection collapse).
//   note: the content-background checkbox sets the content view's background to #99FF0000 / transparent via
//         set_background(solid_paint) — the view base exposes the background as a paint (view.hpp).
//   note: the C# CornerRadiusLayout.IsVisible toggle (corner sliders shown only for RoundRectangle) is
//         reproduced via set_is_visible on the corner-radius section.

#include <cctype>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/border.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class border_playground_page
    {
    public:
        border_playground_page()
        {
            page_.set_title("Borders");
            grid_.set_padding(maui::core::thickness(16)); // twin: <Grid Padding="16">

            // Grid: a 200-unit border row over a star row of scrollable controls.
            grid_.add_row_definition(maui::core::grid_length{200.0});
            grid_.add_row_definition(maui::core::grid_length::star());

            // --- the live border (BorderView) + its initial Label content.
            border_content_label_.set_text("Just a Label");
            // maui-compare BorderPlaygroundPage: the Border content Label is FontSize=20, and each section
            // header uses Headline() = FontSize 18 + Bold. The port set neither (default font); apply both.
            border_content_label_.set_font(maui::core::font::system_font_of_size(20.0));
            for (maui::controls::label* h :
                 {&content_caption_, &shape_caption_, &background_caption_, &content_bg_caption_, &border_caption_,
                  &line_join_caption_, &line_cap_caption_, &corner_caption_})
            {
                h->set_font(maui::core::font::system_font_of_size(18.0, maui::core::font_weight::bold));
            }
            // maui-compare's Info() style = FontSize 8: the small value/color sub-captions. The port had
            // left them at the default label size, which inflates every row and makes the page taller than
            // MAUI (set_caption/wire_corner only rewrite the text, so styling the font up-front sticks).
            for (maui::controls::label* info :
                 {&bg_start_caption_, &bg_end_caption_, &border_start_caption_, &border_end_caption_,
                  &dash_array_caption_, &width_caption_, &dash_offset_caption_, &top_left_caption_, &top_right_caption_,
                  &bottom_left_caption_, &bottom_right_caption_})
            {
                info->set_font(maui::core::font::system_font_of_size(8.0));
            }
            border_view_.set_content(border_content_label_);
            grid_.add(border_view_);
            grid_.set_row(border_view_, 0);

            build_controls();

            scroller_.set_content(controls_);
            grid_.add(scroller_);
            grid_.set_row(scroller_, 1);
            page_.set_content(grid_);

            // C# ctor: seeds the four pickers' SelectedIndex, then runs the Update* pipeline once so the
            // live Border reflects the gradient/shape/dash-array/corner-radius state up front. BUT the
            // shared XAML twin (port/maui-reference/pages/border_playground.xaml) is a STATIC markup
            // snapshot with no code-behind — it authors each Picker with only a Title (no SelectedItem/
            // SelectedIndex) and each color Entry with only Text/Placeholder (no BackgroundColor), so
            // MAUI's actual captured render of the twin shows unselected pickers and plain-background
            // entries, not the C# ctor's imperative seeding. Per the drag_drop/pointer_gesture/
            // shadow_playground precedent this session, the builder's RESTING state must match the twin's
            // static rendering, not the fully-interactive C# ctor's synthetic seed:
            //   - do NOT call set_selected_index on any picker (stays at the picker default -1/unselected,
            //     matching the twin) — every Update* switch on a picker's selected_index() already falls
            //     through its `default:` case to the SAME value the C# seed would have chosen (shape
            //     default -> round_rectangle, line join default -> miter, line cap default -> butt), so
            //     the BORDER's rendered gradient/shape/dash stays byte-identical; only the pickers' own
            //     displayed (non-)selection changes.
            //   - run update_border()/update_background() for their BORDER-repaint side effects (needed
            //     so the live gradient banner + dashed stroke shape render at all — verified to match MAUI
            //     for shape/dash/background; the stroke COLOR is then corrected back to the twin's plain
            //     solid Stroke, see paint_border_only()) but suppress their ENTRY-background-tinting side effect
            //     (BackgroundStartColor/EndColor/BorderStartColor/EndColor.BackgroundColor = color) during
            //     this initial pass — the twin's Entries never carry that tint at rest. Later, genuine
            //     user edits to an Entry's text still run the full update (tint included), matching C#.
            paint_border_only();
            update_content_background();
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::border& border_view()
        {
            return border_view_;
        }
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::picker& shape_picker()
        {
            return shape_picker_;
        }
        [[nodiscard]] maui::controls::slider& width_slider()
        {
            return width_slider_;
        }

    private:
        // Build the bottom-row controls panel and wire each to its C# Update* handler.
        void build_controls()
        {
            controls_.set_spacing(2);

            // Border Content picker (Label / Image).
            content_caption_.set_text("Border Content");
            controls_.add(content_caption_);
            content_picker_.set_title("Border Content"); // twin: <Picker Title="Border Content">
            content_picker_.set_items_source(make_items({"Label", "Image"}));
            content_picker_.selected_index_changed.connect([this] {
                update_border_content();
                update_border();
            });
            controls_.add(content_picker_);

            // Border Shape picker (Rectangle / RoundRectangle / Ellipse).
            shape_caption_.set_text("Border Shape");
            controls_.add(shape_caption_);
            shape_picker_.set_title("Border Shape"); // twin: <Picker Title="Border Shape">
            shape_picker_.set_items_source(make_items({"Rectangle", "RoundRectangle", "Ellipse"}));
            shape_picker_.selected_index_changed.connect([this] { update_border_shape(); });
            controls_.add(shape_picker_);

            // Background gradient color entries.
            background_caption_.set_text("Background");
            controls_.add(background_caption_);
            bg_start_caption_.set_text("Background Start Color");
            controls_.add(bg_start_caption_);
            bg_start_entry_.set_text("#00B4DB");
            bg_start_entry_.set_placeholder("Background Start Color Hex");
            bg_start_entry_.text_changed.connect([this](std::string, std::string) { update_background(); });
            controls_.add(bg_start_entry_);
            bg_end_caption_.set_text("Background End Color");
            controls_.add(bg_end_caption_);
            bg_end_entry_.set_text("#0083B0");
            bg_end_entry_.set_placeholder("Background End Color Hex");
            bg_end_entry_.text_changed.connect([this](std::string, std::string) { update_background(); });
            controls_.add(bg_end_entry_);

            // Content background toggle. Both the true C# source (BorderPlayground.xaml's horizontal
            // StackLayout) and the canonical shared twin (border_playground.xaml) lay the checkbox
            // (WidthRequest=48, VerticalOptions=Center) and its "Show Content Background" label in a
            // HORIZONTAL StackLayout row UNDER the caption — not stacked. Mirror that so the checkbox and
            // its label stay on one compact line (a bare vertical stack detaches them onto two rows).
            content_bg_caption_.set_text("Content Background");
            controls_.add(content_bg_caption_);
            content_bg_check_.checked_changed.connect([this](bool) { update_content_background(); });
            content_bg_check_.set_width_request(48);
            content_bg_check_.set_vertical_layout_alignment(maui::core::layout_alignment::center);
            content_bg_label_.set_text("Show Content Background");
            content_bg_label_.set_vertical_layout_alignment(maui::core::layout_alignment::center);
            content_bg_row_.set_orientation(maui::controls::stack_orientation::horizontal);
            content_bg_row_.add(content_bg_check_);
            content_bg_row_.add(content_bg_label_);
            controls_.add(content_bg_row_);

            // Border gradient color entries.
            border_caption_.set_text("Border");
            controls_.add(border_caption_);
            border_start_caption_.set_text("Border Start Color");
            controls_.add(border_start_caption_);
            border_start_entry_.set_text("#CAC531");
            border_start_entry_.set_placeholder("Border Start Color Hex");
            border_start_entry_.text_changed.connect([this](std::string, std::string) { update_border(); });
            controls_.add(border_start_entry_);
            border_end_caption_.set_text("Border End Color");
            controls_.add(border_end_caption_);
            border_end_entry_.set_text("#F3F9A7");
            border_end_entry_.set_placeholder("Border End Color Hex");
            border_end_entry_.text_changed.connect([this](std::string, std::string) { update_border(); });
            controls_.add(border_end_entry_);

            // Border width slider (Maximum 20, Minimum 0, Value 5) + live readout.
            controls_.add(width_caption_);
            width_slider_.set_minimum(0);
            width_slider_.set_maximum(20);
            width_slider_.set_value(5);
            width_slider_.value_changed.connect([this](double, double v) {
                set_caption(width_caption_, "Border Width: ", v);
                update_border();
            });
            controls_.add(width_slider_);
            set_caption(width_caption_, "Border Width: ", 5);

            // Dash array entry ("1, 1").
            dash_array_caption_.set_text("Border Dash Array:");
            controls_.add(dash_array_caption_);
            dash_array_entry_.set_text("1, 1");
            dash_array_entry_.text_changed.connect([this](std::string, std::string) { update_border(); });
            controls_.add(dash_array_entry_);

            // Dash offset slider (Maximum 2, Minimum 0, Value 1) + live readout.
            controls_.add(dash_offset_caption_);
            dash_offset_slider_.set_minimum(0);
            dash_offset_slider_.set_maximum(2);
            dash_offset_slider_.set_value(1);
            dash_offset_slider_.value_changed.connect([this](double, double v) {
                set_caption(dash_offset_caption_, "Border Dash Offset: ", v);
                update_border();
            });
            controls_.add(dash_offset_slider_);
            set_caption(dash_offset_caption_, "Border Dash Offset: ", 1);

            // Line join picker (Miter / Round / Bevel).
            line_join_caption_.set_text("Border LineJoin");
            controls_.add(line_join_caption_);
            line_join_picker_.set_items_source(make_items({"Miter", "Round", "Bevel"}));
            line_join_picker_.selected_index_changed.connect([this] { update_border_shape(); });
            controls_.add(line_join_picker_);

            // Line cap picker (Butt / Round / Square).
            line_cap_caption_.set_text("Border LineCap");
            controls_.add(line_cap_caption_);
            line_cap_picker_.set_items_source(make_items({"Butt", "Round", "Square"}));
            line_cap_picker_.selected_index_changed.connect([this] { update_border_shape(); });
            controls_.add(line_cap_picker_);

            build_corner_section();
            controls_.add(corner_layout_);
        }

        // The corner-radius section (CornerRadiusLayout): four 0..60 sliders + live readouts.
        void build_corner_section()
        {
            corner_layout_.set_spacing(2);
            corner_caption_.set_text("Corner Radius");
            corner_layout_.add(corner_caption_);

            wire_corner(top_left_caption_, top_left_slider_, "Top Left Corner Radius: ", 20);
            wire_corner(top_right_caption_, top_right_slider_, "Top Right Corner Radius: ", 0);
            wire_corner(bottom_left_caption_, bottom_left_slider_, "Bottom Left Corner Radius: ", 0);
            wire_corner(bottom_right_caption_, bottom_right_slider_, "Bottom Right Corner Radius: ", 12);
        }

        // Wire one corner slider (0..60 at `seed`) + its readout caption into the corner section.
        void wire_corner(maui::controls::label& caption, maui::controls::slider& slider, const char* prefix,
                         double seed)
        {
            corner_layout_.add(caption);
            slider.set_minimum(0);
            slider.set_maximum(60);
            slider.set_value(seed);
            std::string label_prefix = prefix;
            slider.value_changed.connect([this, &caption, label_prefix](double, double v) {
                set_caption(caption, label_prefix.c_str(), v);
                update_corner_radius();
            });
            corner_layout_.add(slider);
            set_caption(caption, prefix, seed);
        }

        // ---- the C# Update* pipeline -----------------------------------------------------------------

        // Paint the Border's own visuals ONCE at construction — the twin's static resting state (header
        // note): the gradient background, the stroke gradient/shape/thickness/dash, all WITHOUT the
        // Update*'s Entry-background-tinting side effect (the twin's Entries never carry that tint at
        // rest; later genuine Entry edits still tint via the normal update_background()/update_border()).
        void paint_border_only()
        {
            update_background(/*tint_entries=*/false);
            update_border(/*tint_entries=*/false);
            // update_border() above always builds a start->end LinearGradientBrush stroke (matching the
            // C# ctor's fully-interactive UpdateBorder body). But the twin's static markup only ever
            // authors `Stroke="#CAC531"` — a plain solid-color string, not a <LinearGradientBrush> (unlike
            // its <Border.Background>, which IS an explicit LinearGradientBrush element) — so the twin's
            // markup, and therefore MAUI's actual captured render of it, shows a SOLID stroke at rest; the
            // gradient only exists once the C# ctor's imperative code runs, which the twin can't express.
            // Measured (windows/{maui,xaml} captures, DIFF_THRESHOLD=25): MAUI's stroke is flat #CAC531
            // (202,197,49) at every sampled x from col 52 to col 954, zero drift; the port's cross-platform
            // border_stroke_spec has no multi-stop-gradient-stroke representation regardless (Gradient
            // strokes are out of scope, core/border_handler.hpp) and collapses ANY gradient paint to
            // gradient_paint::background_color()'s start/end blend — (222,223,108), the exact wrong color
            // this page was showing — so keeping the gradient here could never reach either the twin's nor
            // MAUI's actual pixels. Override the resting stroke back to the twin's real solid color; a
            // later genuine user edit (update_border(true), tint_entries) still builds the full gradient,
            // matching the C# ctor's interactive intent (untested — only the initial frame is captured).
            border_view_.set_stroke(
                std::make_shared<maui::graphics::solid_paint>(color_from_string(border_start_entry_.text())));
        }

        // C# UpdateBackground: a left-to-right linear gradient from the two background-color entries.
        void update_background(bool tint_entries = true)
        {
            const auto start = color_from_string(bg_start_entry_.text());
            const auto end = color_from_string(bg_end_entry_.text());
            border_view_.set_background(make_gradient(start, end));
            if (tint_entries)
            {
                // C# UpdateBackground also tints each color-value Entry's background with its own value
                // (_backgroundStartColor/_backgroundEndColor.BackgroundColor = color) — mirror that.
                bg_start_entry_.set_background(std::make_shared<maui::graphics::solid_paint>(start));
                bg_end_entry_.set_background(std::make_shared<maui::graphics::solid_paint>(end));
            }
        }

        // C# UpdateContentBackground: tint the content view #99FF0000 while checked, else transparent.
        void update_content_background()
        {
            auto* content = border_view_.content();
            if (content == nullptr)
            {
                return;
            }
            const auto tint = content_bg_check_.is_checked() ? maui::graphics::color::from_argb("#99FF0000")
                                                             : maui::graphics::colors::transparent;
            // The content is one of the two owned views; set its background directly.
            if (content == &border_content_label_)
            {
                border_content_label_.set_background(std::make_shared<maui::graphics::solid_paint>(tint));
            }
            else if (content == &border_content_image_)
            {
                border_content_image_.set_background(std::make_shared<maui::graphics::solid_paint>(tint));
            }
        }

        // C# UpdateBorderContent: swap the border's content between the Label and the Image.
        void update_border_content()
        {
            if (content_picker_.selected_index() == 1)
            {
                border_content_image_.set_aspect(maui::core::aspect::aspect_fill);
                border_content_image_.set_source(std::make_shared<maui::controls::file_image_source>("oasis.jpg"));
                border_view_.set_content(border_content_image_);
            }
            else
            {
                border_view_.set_content(border_content_label_);
            }
            update_content_background();
        }

        // C# UpdateBorderShape: the corner sliders are visible only for RoundRectangle, then UpdateBorder.
        void update_border_shape()
        {
            corner_layout_.set_visibility(shape_picker_.selected_index() == 1 ? maui::core::visibility::visible
                                                                              : maui::core::visibility::collapsed);
            update_border();
        }

        // C# UpdateCornerRadius simply re-runs UpdateBorder.
        void update_corner_radius()
        {
            update_border();
        }

        // C# UpdateBorder: rebuild the stroke shape, stroke gradient, thickness, dash array/offset, join, cap.
        void update_border(bool tint_entries = true)
        {
            // Stroke shape from the shape picker.
            switch (shape_picker_.selected_index())
            {
                case 0:
                    border_view_.set_stroke_shape(std::make_shared<maui::graphics::shapes::rectangle>());
                    break;
                case 2:
                    border_view_.set_stroke_shape(std::make_shared<maui::graphics::shapes::ellipse>());
                    break;
                case 1:
                default:
                    border_view_.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(
                        maui::graphics::corner_radius(top_left_slider_.value(), top_right_slider_.value(),
                                                      bottom_left_slider_.value(), bottom_right_slider_.value())));
                    break;
            }

            // Stroke linear gradient from the two border-color entries.
            const auto start = color_from_string(border_start_entry_.text());
            const auto end = color_from_string(border_end_entry_.text());
            border_view_.set_stroke(make_gradient(start, end));
            if (tint_entries)
            {
                // C# UpdateBorder also tints each border-color Entry's background with its own value
                // (_borderStartColor/_borderEndColor.BackgroundColor = color) — mirror that.
                border_start_entry_.set_background(std::make_shared<maui::graphics::solid_paint>(start));
                border_end_entry_.set_background(std::make_shared<maui::graphics::solid_paint>(end));
            }

            // Thickness, dash array (parsed from the entry), dash offset.
            border_view_.set_stroke_thickness(width_slider_.value());
            border_view_.set_stroke_dash_array(parse_dash_array(dash_array_entry_.text()));
            border_view_.set_stroke_dash_offset(dash_offset_slider_.value());

            // Line join.
            switch (line_join_picker_.selected_index())
            {
                case 1:
                    border_view_.set_stroke_line_join(maui::graphics::line_join::round);
                    break;
                case 2:
                    border_view_.set_stroke_line_join(maui::graphics::line_join::bevel);
                    break;
                default:
                    border_view_.set_stroke_line_join(maui::graphics::line_join::miter);
                    break;
            }

            // Line cap (C# Butt → Flat → butt).
            switch (line_cap_picker_.selected_index())
            {
                case 1:
                    border_view_.set_stroke_line_cap(maui::graphics::line_cap::round);
                    break;
                case 2:
                    border_view_.set_stroke_line_cap(maui::graphics::line_cap::square);
                    break;
                default:
                    border_view_.set_stroke_line_cap(maui::graphics::line_cap::butt);
                    break;
            }
        }

        // ---- helpers ---------------------------------------------------------------------------------

        // C# GetColorFromString: empty/invalid hex → transparent, else Color.FromArgb.
        static maui::graphics::color color_from_string(std::string_view value)
        {
            if (value.empty())
            {
                return maui::graphics::colors::transparent;
            }
            maui::graphics::color parsed;
            if (maui::graphics::color::try_parse(value, parsed))
            {
                return parsed;
            }
            return maui::graphics::colors::transparent;
        }

        // A left-to-right linear gradient (start offset 0.0, end offset 0.9) — the C# UpdateBackground/Border
        // gradient brush.
        static std::shared_ptr<maui::graphics::linear_gradient_paint> make_gradient(maui::graphics::color start,
                                                                                    maui::graphics::color end)
        {
            std::vector<maui::graphics::gradient_stop> stops{maui::graphics::gradient_stop(0.0F, start),
                                                             maui::graphics::gradient_stop(0.9F, end)};
            return std::make_shared<maui::graphics::linear_gradient_paint>(
                std::move(stops), maui::graphics::point(0, 0), maui::graphics::point(1, 0));
        }

        // C# DoubleCollectionConverter over the dash-array entry: parse comma/space-separated doubles
        // (empty → an empty pattern, matching the C# branch).
        static std::vector<double> parse_dash_array(std::string_view text)
        {
            std::vector<double> dashes;
            std::string token;
            for (char ch : text)
            {
                if (ch == ',' || std::isspace(static_cast<unsigned char>(ch)) != 0)
                {
                    if (!token.empty())
                    {
                        dashes.push_back(std::stod(token));
                        token.clear();
                    }
                }
                else
                {
                    token.push_back(ch);
                }
            }
            if (!token.empty())
            {
                dashes.push_back(std::stod(token));
            }
            return dashes;
        }

        // Refresh a slider's live readout caption ("<prefix><value>") — the C# {Binding StringFormat} echo.
        static void set_caption(maui::controls::label& caption, const char* prefix, double value)
        {
            char text[64];
            std::snprintf(text, sizeof(text), "%s%g", prefix, value);
            caption.set_text(text);
        }

        // A picker items source from a list of strings.
        static std::shared_ptr<maui::controls::observable_collection<std::string>> make_items(
            std::vector<std::string> values)
        {
            auto source = std::make_shared<maui::controls::observable_collection<std::string>>();
            for (auto& value : values)
            {
                source->add(std::move(value));
            }
            return source;
        }

        maui::controls::content_page page_;
        maui::controls::grid grid_;

        // The live border + its two candidate content views (Label / Image).
        maui::controls::border border_view_;
        maui::controls::label border_content_label_;
        maui::controls::image border_content_image_;

        // The scrollable controls panel.
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout controls_;

        maui::controls::label content_caption_;
        maui::controls::picker content_picker_;
        maui::controls::label shape_caption_;
        maui::controls::picker shape_picker_;
        maui::controls::label background_caption_;
        maui::controls::label bg_start_caption_;
        maui::controls::entry bg_start_entry_;
        maui::controls::label bg_end_caption_;
        maui::controls::entry bg_end_entry_;
        maui::controls::label content_bg_caption_;
        maui::controls::stack_layout content_bg_row_; // twin: <StackLayout Orientation="Horizontal">
        maui::controls::check_box content_bg_check_;
        maui::controls::label content_bg_label_;
        maui::controls::label border_caption_;
        maui::controls::label border_start_caption_;
        maui::controls::entry border_start_entry_;
        maui::controls::label border_end_caption_;
        maui::controls::entry border_end_entry_;
        maui::controls::label width_caption_;
        maui::controls::slider width_slider_;
        maui::controls::label dash_array_caption_;
        maui::controls::entry dash_array_entry_;
        maui::controls::label dash_offset_caption_;
        maui::controls::slider dash_offset_slider_;
        maui::controls::label line_join_caption_;
        maui::controls::picker line_join_picker_;
        maui::controls::label line_cap_caption_;
        maui::controls::picker line_cap_picker_;

        // The corner-radius section (CornerRadiusLayout).
        maui::controls::vertical_stack_layout corner_layout_;
        maui::controls::label corner_caption_;
        maui::controls::label top_left_caption_;
        maui::controls::slider top_left_slider_;
        maui::controls::label top_right_caption_;
        maui::controls::slider top_right_slider_;
        maui::controls::label bottom_left_caption_;
        maui::controls::slider bottom_left_slider_;
        maui::controls::label bottom_right_caption_;
        maui::controls::slider bottom_right_slider_;
    };
} // namespace maui::samples
