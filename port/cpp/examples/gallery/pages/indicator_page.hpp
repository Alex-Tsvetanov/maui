#pragma once
// maui::samples::indicator_page — ports IndicatorPage.xaml
//
// A self-contained, code-first demo of the IndicatorView control. It mirrors the C# gallery page
// (Pages/Controls/IndicatorPage.xaml): a 2-column grid pairing a "Headline" caption label with an
// IndicatorView in each row, walking the whole IndicatorView contract row by row —
//   - Basic:           Count=5, Position=2 (the default circular dots);
//   - Colors:          SelectedIndicatorColor=Red, IndicatorColor=Blue, Background=Yellow;
//   - Indicator Shape: IndicatorsShape=Square;
//   - Indicator Size:  IndicatorSize=15;
//   - Indicator HideSingle: Count=1 + HideSingle=true (a single dot stays hidden);
//   - Indicator MaximumVisible: Count=10, Position=3, MaximumVisible=7 ("7 of 10");
//   - Indicator Template: the C# custom IndicatorTemplate (an Ionicons glyph Label) — and
//   - Using with CarouselView: a CarouselView of three string items wired to an IndicatorView so
//     swiping the carousel advances the dots.
//
// The page OWNS its whole element tree (the grid_page / sample_app pattern). It is backend-agnostic —
// a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT COLLAPSES (faithful best-effort + note, never invented):
//   note: the C# IndicatorView is a TemplatedView that can host a custom IndicatorTemplate (the
//         "Indicator Template" row's Ionicons glyph and the CarouselView row's {Binding} Label). The
//         port's indicator_view OMITS the template path (indicator_view.hpp header) and renders the
//         default dots only, so the two template rows are ported as default-dot indicators (the
//         transparent IndicatorColor/SelectedIndicatorColor of the C# template row is preserved on
//         the first, which leaves it effectively invisible — exactly the C# intent of letting the
//         template glyph show instead of the dots).
//   note: the C# CarouselView links its indicator declaratively (CarouselView IndicatorView=
//         "indicatorView"), which keeps Position + ItemsSource synced automatically. The port has no
//         IndicatorView linkage property on carousel_view, so the wiring is reproduced explicitly:
//         the carousel's items seed the indicator's Count (set_items_source over the same strings),
//         and the carousel's position_changed pushes the new position into the indicator
//         (set_position_manual) — the same observable behavior the declarative link produces.
//   note: the C# Labels/IndicatorViews use HorizontalOptions="Center" / VerticalOptions="Start". The
//         port's view base exposes no layout-options setter (the grid_page precedent), so the caption
//         labels center via horizontal TEXT alignment instead — the closest headless-safe equivalent.
//   note: the C# Background="Yellow" on the Colors-row IndicatorView is ported through the view
//         background paint (a solid_paint over yellow), the cross-platform equivalent of the XAML
//         Background brush.

#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/indicator_shape.hpp"
#include "maui/controls/indicator_view.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class indicator_page
    {
    public:
        indicator_page()
        {
            page_.set_title("IndicatorView");

            // The shared indicator.xaml root is <Grid Padding="12" RowSpacing="6" ColumnSpacing="6"> — set
            // the same so the code-first column's row positions line up with the maui/xaml columns (without
            // this the rows pack ~6pt tighter each and the whole grid rides ~12pt higher).
            grid_.set_padding(maui::core::thickness(12));
            grid_.set_row_spacing(6);
            grid_.set_column_spacing(6);

            // Eight rows (seven 50-unit caption/indicator rows + a star row for the carousel block);
            // two columns (caption | indicator) — the C# Grid.RowDefinitions/ColumnDefinitions.
            for (int row = 0; row < 7; ++row)
            {
                grid_.add_row_definition(maui::core::grid_length{50.0});
            }
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());

            // Row 0 — Basic: Count=5, Position=2.
            caption(basic_label_, "Basic", 0);
            basic_indicator_.set_count(5);
            basic_indicator_.set_position_manual(2);
            place(basic_indicator_, 0);

            // Row 1 — Colors: Red selected / Blue unselected / Yellow background.
            caption(colors_label_, "Colors", 1);
            colors_indicator_.set_count(5);
            colors_indicator_.set_position_manual(2);
            colors_indicator_.set_selected_indicator_color(maui::graphics::colors::red);
            colors_indicator_.set_indicator_color(maui::graphics::colors::blue);
            colors_indicator_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::yellow));
            place(colors_indicator_, 1);

            // Row 2 — Indicator Shape: Square.
            caption(shape_label_, "Indicator Shape", 2);
            shape_indicator_.set_count(5);
            shape_indicator_.set_position_manual(2);
            shape_indicator_.set_indicators_shape(maui::controls::indicator_shape::square);
            place(shape_indicator_, 2);

            // Row 3 — Indicator Size: 15.
            caption(size_label_, "Indicator Size", 3);
            size_indicator_.set_count(5);
            size_indicator_.set_position_manual(2);
            size_indicator_.set_indicator_size(15);
            place(size_indicator_, 3);

            // Row 4 — Indicator HideSingle: Count=1 + HideSingle=true (a lone dot stays hidden).
            caption(hide_single_label_, "Indicator HideSingle", 4);
            hide_single_indicator_.set_count(1);
            hide_single_indicator_.set_hide_single(true);
            place(hide_single_indicator_, 4);

            // Row 5 — Indicator MaximumVisible: 7 of 10 (Count=10, Position=3, MaximumVisible=7).
            caption(max_visible_label_, "Indicator MaximumVisible -  7 of 10", 5);
            max_visible_indicator_.set_count(10);
            max_visible_indicator_.set_position_manual(3);
            max_visible_indicator_.set_maximum_visible(7);
            place(max_visible_indicator_, 5);

            // Row 6 — Indicator Template: the C# custom IndicatorTemplate row. The template path is
            // omitted in the port (header note), so this renders default dots; the C# transparent
            // colors are preserved (the template-glyph-instead-of-dots intent).
            caption(template_label_, "Indicator Template", 6);
            template_indicator_.set_count(5);
            template_indicator_.set_position_manual(2);
            template_indicator_.set_indicator_color(maui::graphics::colors::transparent);
            template_indicator_.set_selected_indicator_color(maui::graphics::colors::transparent);
            place(template_indicator_, 6);

            // Row 7 — Using with CarouselView: a 3-item string carousel + an indicator, wired together.
            caption(carousel_label_, "Using with CarouselView", 7);
            carousel_label_.set_vertical_text_alignment(maui::core::text_alignment::start);

            const std::vector<std::string> items{"Item 1", "Item 2", "Item 3"};
            carousel_.set_items_source(items);
            carousel_.set_loop(false);
            carousel_.set_height_request(100);

            // The C# IndicatorView="indicatorView" link, reproduced explicitly (header note): seed the
            // indicator's Count from the same items and push the carousel position into it on swipe.
            carousel_indicator_.set_items_source(items);
            carousel_indicator_.set_indicator_color(maui::graphics::colors::light_gray);
            carousel_indicator_.set_selected_indicator_color(maui::graphics::colors::dark_gray);
            carousel_.position_changed.connect([this](const maui::controls::position_changed_event_args& args) {
                carousel_indicator_.set_position_manual(args.current_position);
            });

            grid_.add(carousel_);
            grid_.set_row(carousel_, 7);
            grid_.set_column(carousel_, 1);
            grid_.add(carousel_indicator_);
            grid_.set_row(carousel_indicator_, 7);
            grid_.set_column(carousel_indicator_, 1);

            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::indicator_view& basic_indicator()
        {
            return basic_indicator_;
        }
        [[nodiscard]] maui::controls::carousel_view& carousel()
        {
            return carousel_;
        }
        [[nodiscard]] maui::controls::indicator_view& carousel_indicator()
        {
            return carousel_indicator_;
        }

    private:
        // One "Headline"-style caption label in the left column of a row (centered TEXT alignment stands
        // in for the C# HorizontalOptions="Center" the port can't express on the view — header note).
        void caption(maui::controls::label& text, const char* value, int row)
        {
            text.set_text(value);
            text.set_horizontal_text_alignment(maui::core::text_alignment::center);
            // Each caption is FontAttributes="Bold" in the shared indicator.xaml — match it (default size,
            // bold weight) so the code-first column agrees with the maui/xaml columns.
            text.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::bold));
            grid_.add(text);
            grid_.set_row(text, row);
            grid_.set_column(text, 0);
        }
        // Place one IndicatorView in the right column of a row.
        void place(maui::controls::indicator_view& indicator, int row)
        {
            grid_.add(indicator);
            grid_.set_row(indicator, row);
            grid_.set_column(indicator, 1);
        }

        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::label basic_label_;
        maui::controls::indicator_view basic_indicator_;
        maui::controls::label colors_label_;
        maui::controls::indicator_view colors_indicator_;
        maui::controls::label shape_label_;
        maui::controls::indicator_view shape_indicator_;
        maui::controls::label size_label_;
        maui::controls::indicator_view size_indicator_;
        maui::controls::label hide_single_label_;
        maui::controls::indicator_view hide_single_indicator_;
        maui::controls::label max_visible_label_;
        maui::controls::indicator_view max_visible_indicator_;
        maui::controls::label template_label_;
        maui::controls::indicator_view template_indicator_;
        maui::controls::label carousel_label_;
        maui::controls::carousel_view carousel_;
        maui::controls::indicator_view carousel_indicator_;
    };
} // namespace maui::samples
