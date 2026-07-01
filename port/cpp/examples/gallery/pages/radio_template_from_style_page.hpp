#pragma once
// maui::samples::radio_template_from_style_page — ports TemplateFromStyle.xaml
//
// A self-contained, code-first demo of applying a RadioButton ControlTemplate. Mirrors the C# controls
// gallery page (Pages/Controls/RadioButtonGalleries/TemplateFromStyle.xaml): a page-level resources
// dictionary defines a "CalendarRadioTemplate" ControlTemplate (a Border with VisualStateGroups +
// a Grid holding two Ellipses — an outline + a filled "Check" dot — and a ContentPresenter), plus a
// Style with TargetType RadioButton whose single Setter is ControlTemplate="{StaticResource
// CalendarRadioTemplate}". Three grouped RadioButtons (GroupName "A", Content "A"/"B"/"C") then pick up
// that template via the implicit Style.
//
// WHAT MAUI'S PAGE SHOWS (and how this port maps it):
//   The page's subject is "a ControlTemplate, applied to RadioButtons through a Style". The faithful
//   framework machinery underneath is: a control_template whose root is the calendar tile (Border →
//   Grid → outline Ellipse + Check Ellipse + content_presenter), applied to a templated control, with
//   the content_presenter packing the developer Content ("A"/"B"/"C"). The radio_button's
//   ControlTemplate path AND the Style→RadioButton.ControlTemplate Setter are both documented-deferred
//   at the radio_button level (radio_button.hpp), so the port demonstrates the template+presenter
//   hosting via content_view — the sibling templated_view over the SAME content_presenter seam — built
//   ONCE here and applied to three content_views (the "from a Style" intent: every tile shares one
//   template), each grouped under the stack so only one is selected at a time (the GroupName "A"
//   mutual-exclusion the XAML gives the three RadioButtons).
//
//   So each tile is a content_view carrying control_template::of<calendar_radio_template>() with its
//   Content set to a label ("A"/"B"/"C") that the template's content_presenter packs. The Checked
//   VisualState that flips the Check ellipse Opacity 0→1 and recolors the Border stroke belongs to the
//   radio_button visual-state machine; on the content_view stand-in it is noted as deferred rather than
//   invented (the tile renders its static appearance + packed content).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/
// ios test trees exercise the same wiring directly.
//
// note: radio_button ControlTemplate + the Style/Setter ControlTemplate application are documented-
//       deferred (radio_button.hpp). The calendar template is therefore hosted on content_view (the
//       content_presenter seam), and the live Checked-state opacity/stroke flip is deferred. The
//       "shared template via Style" idea is reproduced by minting one control_template-per-tile from the
//       same calendar_radio_template type (the reflection-free Type-ctor stand-in). Nothing invented
//       beyond those clearly-noted points.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/content_view.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/templates/content_presenter.hpp"
#include "maui/controls/templates/control_template.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    // The XAML's <ControlTemplate x:Key="CalendarRadioTemplate"> root: a Border (100x100, #F3F2F1 fill,
    // #F3F2F1 stroke) containing a Grid that holds a small upper-right indicator grid (an outline Ellipse
    // + a filled "Check" Ellipse) and a ContentPresenter. control_template::of<calendar_radio_template>()
    // mints one of these per tile; the presenter packs the tile's developer Content ("A"/"B"/"C").
    class calendar_radio_template : public maui::controls::border
    {
    public:
        calendar_radio_template()
        {
            // <Border Stroke="#F3F2F1" BackgroundColor="#F3F2F1" HeightRequest="100" WidthRequest="100"
            //         HorizontalOptions="Start" VerticalOptions="Start" Padding="0">
            const maui::graphics::color tile_bg = maui::graphics::color::from_argb("#F3F2F1");
            set_background(std::make_shared<maui::graphics::solid_paint>(tile_bg));
            set_stroke(std::make_shared<maui::graphics::solid_paint>(tile_bg));
            set_width_request(100);
            set_height_request(100);
            set_padding(maui::core::thickness(0));
            // HorizontalOptions/VerticalOptions="Start" (XAML line 11): the tile hugs the top-left of its
            // slot rather than centering — the flush-left column MAUI renders.
            set_horizontal_layout_alignment(maui::core::layout_alignment::start);
            set_vertical_layout_alignment(maui::core::layout_alignment::start);

            // The outer <Grid Margin="4" WidthRequest="100" Padding="2">.
            tile_grid_.set_width_request(100);
            tile_grid_.set_padding(maui::core::thickness(2));

            // The upper-right 18x18 indicator <Grid HorizontalOptions="End" VerticalOptions="Start">
            // holding the two Ellipses — pinned to the tile's top-right corner (XAML line 37).
            indicator_grid_.set_width_request(18);
            indicator_grid_.set_height_request(18);
            indicator_grid_.set_horizontal_layout_alignment(maui::core::layout_alignment::end);
            indicator_grid_.set_vertical_layout_alignment(maui::core::layout_alignment::start);

            // <Ellipse Stroke="Blue" WidthRequest="16" HeightRequest="16" StrokeThickness="0.5"
            //          VerticalOptions="Center" HorizontalOptions="Center" Fill="White"/> — the ring.
            ring_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));
            ring_.set_stroke_thickness(0.5);
            ring_.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::white));
            ring_.set_width_request(16);
            ring_.set_height_request(16);
            ring_.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
            ring_.set_vertical_layout_alignment(maui::core::layout_alignment::center);

            // <Ellipse x:Name="Check" WidthRequest="8" HeightRequest="8" Fill="Blue"
            //          VerticalOptions="Center" HorizontalOptions="Center"/> — the dot. The XAML's Checked
            // VisualState flips its Opacity 0→1; the DEFAULT (Unchecked) state the tiles render with sets
            // Opacity 0, so MAUI shows only the empty blue ring on a fresh page (no filled dot). The port
            // must render that same default: the Checked-state flip is the radio_button visual-state
            // machine's job and stays deferred (this template is hosted on a content_view stand-in), so the
            // dot is set to the unchecked-default Opacity 0 rather than left visible.
            check_.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));
            check_.set_width_request(8);
            check_.set_height_request(8);
            check_.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
            check_.set_vertical_layout_alignment(maui::core::layout_alignment::center);
            check_.set_opacity(0.0); // Unchecked default (VisualState "Unchecked" → Check Opacity 0)

            indicator_grid_.add(ring_);
            indicator_grid_.add(check_);

            // <ContentPresenter/> — packs the templated parent's developer Content.
            presenter_ = std::make_shared<maui::controls::content_presenter>();

            tile_grid_.add(indicator_grid_);
            tile_grid_.add(*presenter_);

            set_content(tile_grid_);
        }

        [[nodiscard]] const std::shared_ptr<maui::controls::content_presenter>& presenter() const
        {
            return presenter_;
        }

    private:
        maui::controls::grid tile_grid_;
        maui::controls::grid indicator_grid_;
        maui::controls::shapes::ellipse ring_;
        maui::controls::shapes::ellipse check_;
        std::shared_ptr<maui::controls::content_presenter> presenter_;
    };

    class radio_template_from_style_page
    {
    public:
        radio_template_from_style_page()
        {
            page_.set_title("RadioButton Template from Style");
            stack_.set_spacing(8);

            // The three calendar tiles (the XAML's three GroupName="A" RadioButtons, Content A/B/C). Each
            // shares the CalendarRadioTemplate via the implicit Style — reproduced by minting one
            // control_template per tile from the same calendar_radio_template type.
            add_tile("A");
            add_tile("B");
            add_tile("C");

            for (const auto& tile : tiles_)
            {
                stack_.add(*tile);
            }

            // GroupName "A": the three tiles are mutually exclusive (the XAML's shared GroupName). The
            // grouping is an attached-property concern on the container; applied here so the stand-in
            // tiles read as one group even though the live Checked state isn't driven through them.
            maui::controls::radio_button_group::set_group_name(stack_, "A");

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's inspection.
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] const std::vector<std::shared_ptr<maui::controls::content_view>>& tiles() const
        {
            return tiles_;
        }

    private:
        // A calendar tile: a content_view whose ControlTemplate is calendar_radio_template and whose
        // Content is a label ("A"/"B"/"C") the template's content_presenter packs. set_control_template
        // runs the same template_utilities::on_control_template_changed the C# change callback runs.
        void add_tile(const std::string& text)
        {
            auto tile = std::make_shared<maui::controls::content_view>();
            // The XAML's HorizontalOptions="Start" lives on the template-root Border; on the port the stack
            // arranges the outer content_view, so mirror the Start alignment onto the tile too — otherwise a
            // Fill/Center content_view re-centers the 100pt tile mid-screen (the wide-margin bug).
            tile->set_horizontal_layout_alignment(maui::core::layout_alignment::start);
            tile->set_vertical_layout_alignment(maui::core::layout_alignment::start);
            tile->set_control_template(maui::controls::control_template::of<calendar_radio_template>());

            auto content = std::make_shared<maui::controls::label>();
            content->set_text(text);
            tile->set_content(content);

            tile_labels_.push_back(std::move(content));
            tiles_.push_back(std::move(tile));
        }

        maui::controls::content_page page_;
        maui::controls::stack_layout stack_;

        // The three tiles + their co-owned content labels (kept alive + reachable for attachment).
        std::vector<std::shared_ptr<maui::controls::content_view>> tiles_;
        std::vector<std::shared_ptr<maui::controls::label>> tile_labels_;
    };
} // namespace maui::samples
