#pragma once
// maui::controls::button_content_layout  <=  Microsoft.Maui.Controls.Button.ButtonContentLayout
//
// The image/text composition spec for a button: where the image sits relative to the text
// (image_position) and the spacing between them. Ported from the sealed nested class
// Button.ButtonContentLayout (Button.cs:528-567): an immutable (position, spacing) pair with the
// ImagePosition enum {Left, Top, Right, Bottom}.
//
// In MAUI this drives the native text+image composition; in the port it is STORED + PUSHED to the
// handler (Button.ContentLayout change → InvalidateMeasureInternal), but the actual measure/arrange
// composition is deferred (no container infrastructure on any backend yet — see button_handler).
//
// Equality is value-based so the bindable_property change detection fires only on a genuine change
// (C#'s ButtonContentLayout is a reference type, but the property setter compares the new instance —
// the port models the intended value semantics directly).

namespace maui::controls
{
    struct button_content_layout
    {
        // C# Button.ButtonContentLayout.ImagePosition — the image's side relative to the text.
        enum class image_position
        {
            left,
            top,
            right,
            bottom
        };

        // Button.cs DefaultSpacing = 10.
        static constexpr double default_spacing = 10.0;

        image_position position = image_position::left;
        double spacing = default_spacing;

        button_content_layout() = default;
        button_content_layout(image_position position_value, double spacing_value)
            : position(position_value), spacing(spacing_value)
        {
        }
    };

    inline bool operator==(const button_content_layout& a, const button_content_layout& b)
    {
        return a.position == b.position && a.spacing == b.spacing;
    }
    inline bool operator!=(const button_content_layout& a, const button_content_layout& b)
    {
        return !(a == b);
    }
} // namespace maui::controls
