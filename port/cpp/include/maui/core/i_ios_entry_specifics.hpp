#pragma once
// maui::core::i_ios_entry_specifics  <=  (port seam) Microsoft.Maui.Controls.Entry's iOS remap surface
//
// C# wires the iOSSpecific Entry.CursorColor knob by APPENDING a Controls-layer mapping to the Core
// EntryHandler (Entry.Mapper.cs ReplaceMapping("CursorColor", Entry.iOS.cs MapCursorColor →
// TextExtensions.UpdateCursorColor reading the typed Entry). The port's mapper tables are Core-owned,
// so the value crosses on an IiOSPageSpecifics-style side contract instead: controls::entry implements
// it over the platform-spec store; the per-backend map body dynamic_casts the i_entry to it (W2-24).

#include <optional>

#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_ios_entry_specifics
    {
    public:
        virtual ~i_ios_entry_specifics() = default;

        // C# entry.IsSet(iOSSpecific.Entry.CursorColorProperty) — UpdateCursorColor's outer guard (an
        // untouched entry must not clobber the field's default tint).
        [[nodiscard]] virtual bool cursor_color_set() const = 0;

        // C# iOSSpecific.Entry.GetCursorColor — nullopt is C#'s null Color (UpdateCursorColor skips it).
        [[nodiscard]] virtual std::optional<maui::graphics::color> cursor_color() const = 0;

    protected:
        i_ios_entry_specifics() = default;
        i_ios_entry_specifics(const i_ios_entry_specifics&) = default;
        i_ios_entry_specifics(i_ios_entry_specifics&&) = default;
        i_ios_entry_specifics& operator=(const i_ios_entry_specifics&) = default;
        i_ios_entry_specifics& operator=(i_ios_entry_specifics&&) = default;
    };
} // namespace maui::core
