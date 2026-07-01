#pragma once
// maui::samples::flex_layout_page — ports FlexLayoutPage.xaml
//
// A self-contained, code-first demo of the FlexLayout control: the classic "holy grail" page layout built
// from nested flexboxes. It mirrors the C# gallery page (Pages/Layouts/FlexLayoutPage.xaml):
//   - an OUTER FlexLayout with Direction="Column": a HEADER label, a BODY flex, and a FOOTER label stacked
//     vertically; the body carries FlexLayout.Grow="1" so it soaks up all the leftover vertical space while
//     the header/footer stay at their content height;
//   - the BODY is a (default Direction=Row) FlexLayout holding three children: a CONTENT label with
//     FlexLayout.Grow="1" (it fills the row's slack), a NAVIGATION BoxView with FlexLayout.Basis="50" and
//     FlexLayout.Order="-1" (a 50-unit-wide bar forced to the FRONT of the row regardless of source order),
//     and an ASIDE BoxView with FlexLayout.Basis="50" (a 50-unit-wide bar at the end).
// It demonstrates the container knob (Direction) plus the per-child attached values Grow, Basis and Order,
// and the nesting of one flex inside another (a flex that is itself a flex child).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// note: the C# labels use FontSize="Large", a device-dependent NAMED size; the port has no named-size
//       resolver, so a fixed 18-pt system font stands in for "Large" (the conventional resolved value) — a
//       faithful best-effort, not the platform-exact metric. HorizontalTextAlignment/VerticalTextAlignment
//       are ported directly.

#include <memory>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/flex_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/font.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/layouts/flex_basis.hpp"
#include "maui/layouts/flex_enums.hpp"

namespace maui::samples
{
    class flex_layout_page
    {
    public:
        flex_layout_page()
        {
            page_.set_title("FlexLayout");

            // "Large" font size is a device-dependent named size; 18pt stands in (see header note).
            const auto large_font = maui::core::font::system_font_of_size(18.0);

            // The C# XAML sets NO explicit TextColor on these labels, so MAUI renders them in the
            // platform DEFAULT label text color — on Android that is a dark neutral gray (measured
            // #3B3B3B from docs/comparison/android/maui/flex_layout.png). The port's DeviceDefault
            // theme resolves the unset text color to a much LIGHTER bluish-gray (~#74757F), which is
            // near-invisible on this page's gray CONTENT region. Pin the dark default explicitly on
            // these labels (page-local — NOT a global theme change) so they read on the colored
            // FlexLayout regions exactly as MAUI does. See PROFILE parity policy §1 (MAUI is truth).
            const auto default_label_text = maui::graphics::color::from_rgb(59, 59, 59); // #3B3B3B

            // Outer flex: a vertical column (header / body / footer).
            root_.set_direction(maui::layouts::flex_direction::column);

            // HEADER — an aqua title bar at content height.
            header_.set_text("HEADER");
            header_.set_font(large_font);
            header_.set_text_color(default_label_text);
            header_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::aqua));
            header_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            root_.add(header_);

            // BODY — a (default Row) flex that grows to fill the column's leftover height.
            //   FlexLayout.Grow="1" is an attached value the OUTER flex stores for the body child.
            //   The body's own three children (content / nav / aside) get their attached values from BODY.
            content_.set_text("CONTENT");
            content_.set_font(large_font);
            content_.set_text_color(default_label_text);
            content_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::gray));
            content_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            content_.set_vertical_text_alignment(maui::core::text_alignment::center);
            body_.add(content_);
            body_.set_grow(content_, 1.0F); // FlexLayout.Grow="1" — content fills the row's slack

            // Navigation bar — 50-unit basis, Order=-1 forces it to the FRONT of the row.
            nav_.set_color(maui::graphics::colors::blue);
            body_.add(nav_);
            body_.set_basis(nav_, maui::layouts::flex_basis{50.0F, false}); // FlexLayout.Basis="50" (absolute)
            body_.set_order(nav_, -1);                                      // FlexLayout.Order="-1"

            // Aside bar — 50-unit basis, default order (stays at the end).
            aside_.set_color(maui::graphics::colors::green);
            body_.add(aside_);
            body_.set_basis(aside_, maui::layouts::flex_basis{50.0F, false}); // FlexLayout.Basis="50" (absolute)

            root_.add(body_);
            root_.set_grow(body_, 1.0F); // FlexLayout.Grow="1" on the body within the outer column

            // FOOTER — a pink bar at content height.
            footer_.set_text("FOOTER");
            footer_.set_font(large_font);
            footer_.set_text_color(default_label_text);
            footer_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::pink));
            footer_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            root_.add(footer_);

            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::flex_layout& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::flex_layout& body()
        {
            return body_;
        }
        [[nodiscard]] maui::controls::label& header()
        {
            return header_;
        }
        [[nodiscard]] maui::controls::label& content()
        {
            return content_;
        }
        [[nodiscard]] maui::controls::label& footer()
        {
            return footer_;
        }
        [[nodiscard]] maui::controls::box_view& nav()
        {
            return nav_;
        }
        [[nodiscard]] maui::controls::box_view& aside()
        {
            return aside_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::flex_layout root_; // outer column: header / body / footer
        maui::controls::flex_layout body_; // inner row: content / nav / aside
        maui::controls::label header_;
        maui::controls::label content_;
        maui::controls::label footer_;
        maui::controls::box_view nav_;
        maui::controls::box_view aside_;
    };
} // namespace maui::samples
