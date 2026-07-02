#pragma once
// maui::core::i_text_button  <=  Microsoft.Maui.ITextButton
//
// A button that displays text — the union of i_button and i_text. Ported from
// src/Core/src/Core/ITextButton.cs (ITextButton : IView, IButton, IText). This is the virtual-view
// type the Button control implements and the type the handler's text mapper is keyed on (C#'s
// TextButtonMapper<ITextButton>): keying on i_text_button (rather than i_text) keeps the mapper's
// Virtual an i_element, as the property_mapper requires. No diamond — i_view arrives only via i_button,
// i_text_style only via i_text.

#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_text.hpp"

namespace maui::core
{
    // The measure-relevant projection of Button.ContentLayout (position + spacing) — the two scalars the
    // iOS CrossPlatformMeasure needs to know whether the image/title compose along the WIDTH axis
    // (Left/Right) or the HEIGHT axis (Top/Bottom), and by how much spacing. Kept as a bare POD in the
    // CORE layer (i_text_button lives in maui::core and must not depend on maui::controls, where the richer
    // maui::controls::button_content_layout lives). image_position mirrors
    // Button.ButtonContentLayout.ImagePosition {Left, Top, Right, Bottom} 1:1 by ordinal, so the concrete
    // Button converts by ordinal. NARROW, like image_source(): only the measure path reads it.
    struct button_content_spec
    {
        enum class image_position
        {
            left,
            top,
            right,
            bottom
        };
        image_position position = image_position::left;
        double spacing = 10.0; // Button.cs DefaultSpacing = 10.
    };

    class i_text_button : public i_button, public i_text
    {
    public:
        // The button's image source (C# Button.ImageSource, surfaced through IImage.Source for the
        // cross-platform ImageButtonMapper). NARROW: rather than make Button an i_image (the diamond
        // i_text + i_image would create — both derive i_view, forbidden by PROFILE.md), the single read
        // the handler's image mapper needs lives here on i_text_button (the Button's own virtual-view
        // type). A raw borrow: the control owns the source (a shared_ptr); null means "no image". The
        // ContentLayout value stays control-side — its mapper only re-measures (deferred), so the handler
        // never reads it (see button_handler::map_content_layout). Defaulted to null so the only concrete
        // i_text_button (Button) opts in and no future implementer is forced to carry an image.
        [[nodiscard]] virtual maui::core::i_image_source* image_source() const
        {
            return nullptr;
        }

        // The button's ContentLayout, projected to the measure-relevant (position, spacing) POD. NARROW,
        // like image_source(): C#'s Button.iOS.cs CrossPlatformMeasure reads button.ContentLayout to decide
        // the image/title composition axis (Left/Right → width, Top/Bottom → height) and the spacing between
        // them, and the iOS get_desired_size ports that measure — so it needs this read. The value stays
        // owned control-side (Button.ContentLayout); this is a pure read. Defaulted to the C# default
        // ButtonContentLayout (Left, spacing 10) so a non-image or default button behaves exactly as MAUI's
        // default, and no future i_text_button implementer is forced to carry ContentLayout.
        [[nodiscard]] virtual maui::core::button_content_spec content_layout_spec() const
        {
            return {};
        }

        // C# Button.IImageSourcePart.UpdateIsLoading (Button.cs:499-505): the handler's image mapper pushes
        // the loader's in-flight loading state here so a load FINISH re-pushes ContentLayout (re-measure).
        // NARROW like image_source(): the seam lives on i_text_button rather than widening Button to i_image.
        // Defaulted to a no-op so non-image i_text_button implementers need not carry it; Button overrides.
        virtual void update_is_loading(bool /*is_loading*/)
        {
        }
    };
} // namespace maui::core
