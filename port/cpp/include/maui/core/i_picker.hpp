#pragma once
// maui::core::i_picker  <=  Microsoft.Maui.IPicker
//
// The virtual-view contract for a single-selection item picker. Ported from
// src/Core/src/Core/IPicker.cs (IPicker : IView, ITextStyle, ITextAlignment, IItemDelegate<string>).
//
// The items themselves cross the seam ONLY through the i_item_delegate face (get_count/get_item) —
// the platform recipes (PickerExtensions.UpdatePicker, PickerSource) read items exclusively through
// it; C#'s `IList<string> Items` surface lives on the control (maui::controls::picker::items()).
//
// selected_index has BOTH accessors because the selection flows both ways: the mapper pushes the
// virtual value to the native picker, and a native row pick writes back through set_selected_index
// (the control stores it at setter_specificity::from_handler, like C#'s explicit IPicker.SelectedIndex
// setter using SetterSpecificity.FromHandler).
//
// Deferred (documented, not stubbed): IsOpen + the Opened/Closed events (the focus/first-responder
// subsystem is out of the port's contract — see picker.hpp).

#include <string>
#include <string_view>

#include "maui/core/i_item_delegate.hpp"
#include "maui/core/i_text_alignment.hpp"
#include "maui/core/i_text_style.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_picker : public i_view, public i_text_style, public i_text_alignment, public i_item_delegate<std::string>
    {
    public:
        [[nodiscard]] virtual std::string_view title() const = 0;
        [[nodiscard]] virtual maui::graphics::color title_color() const = 0;

        [[nodiscard]] virtual int selected_index() const = 0;
        // Inbound channel: the handler writes the native row pick back (FromHandler specificity).
        virtual void set_selected_index(int value) = 0;
    };
} // namespace maui::core
