#pragma once
// maui::graphics::text::text_attributes  <=  Microsoft.Maui.Graphics.Text.TextAttributes
//   (+ TextAttribute, MarkerType — the attribute enums, a tight cluster per PROFILE §3 —
//    + the typed accessors from TextAttributesExtensions.cs / TextAttributeExtensions.cs)
//
// A string-keyed attribute bag for a run of attributed text. C# models this as
// Dictionary<TextAttribute, string> implementing ITextAttributes (an IReadOnlyDictionary view);
// the port collapses that interface into this one concrete map type — TextAttributes is its only
// implementation and value semantics are what attributed_text_run needs. The C# extension-method
// accessors (GetBoolAttribute / SetFontSize / ...) become members here (free-function extensions
// fold into the type they extend when there is exactly one receiver type).
//
// Values stay strings exactly as in C# (the XML reader/writer round-trips them; bools are stored as
// C# bool.ToString() "True"/"False", numbers invariant-culture). Out-of-line definitions live in
// src/graphics/text/text_attributes.cpp.

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace maui::graphics::text
{
    // Microsoft.Maui.Graphics.Text.TextAttribute.
    enum class text_attribute : std::uint8_t
    {
        font_name = 0,
        font_size,
        bold,
        italic,
        underline,
        strikethrough,
        subscript,
        superscript,
        color,
        background,
        unordered_list,
        marker
    };

    // Microsoft.Maui.Graphics.Text.MarkerType.
    enum class marker_type : std::uint8_t
    {
        closed_circle = 0,
        open_circle = 1,
        hyphen = 2
    };

    class text_attributes
    {
    public:
        // C# TextAttributes() — empty bag.
        text_attributes() = default;
        // C# TextAttributes(first, second) — the union; entries of `second` win on key collisions
        // (TextAttributeExtensions.Union routes here).
        text_attributes(const text_attributes& first, const text_attributes& second);

        // ---- the IReadOnlyDictionary surface ----
        [[nodiscard]] std::size_t size() const;
        [[nodiscard]] bool empty() const;
        [[nodiscard]] bool contains(text_attribute type) const;
        [[nodiscard]] const std::map<text_attribute, std::string>& entries() const;

        // ---- raw accessors (TextAttributesExtensions) ----
        // C# GetAttribute(type, defaultValue = null) — nullopt stands in for C#'s null string.
        [[nodiscard]] std::optional<std::string> get_attribute(text_attribute type) const;
        [[nodiscard]] std::string get_attribute(text_attribute type, std::string_view default_value) const;
        // C# SetAttribute(type, value) — a null value removes; the port spells that remove_attribute.
        void set_attribute(text_attribute type, std::string value);
        void remove_attribute(text_attribute type);

        // ---- typed accessors (TextAttributesExtensions; setters remove when value == default) ----
        [[nodiscard]] int get_int_attribute(text_attribute type, int default_value) const;
        void set_int_attribute(text_attribute type, int value, int default_value);
        [[nodiscard]] float get_float_attribute(text_attribute type, float default_value) const;
        void set_float_attribute(text_attribute type, float value, float default_value);
        [[nodiscard]] bool get_bool_attribute(text_attribute type, bool default_value = false) const;
        void set_bool_attribute(text_attribute type, bool value, bool default_value = false);

        // ---- well-known attributes (TextAttributeExtensions) ----
        // C# TextAttributeExtensions.DefaultFontSize.
        static constexpr float default_font_size = 12.0F;

        [[nodiscard]] std::optional<std::string> get_font_name() const;
        void set_font_name(const std::string& value);
        [[nodiscard]] float get_font_size(std::optional<float> font_size = std::nullopt) const;
        void set_font_size(float value);
        [[nodiscard]] bool get_underline() const;
        void set_underline(bool value);
        [[nodiscard]] bool get_bold() const;
        void set_bold(bool value);
        [[nodiscard]] bool get_italic() const;
        void set_italic(bool value);
        [[nodiscard]] bool get_unordered_list() const;
        void set_unordered_list(bool value);
        // C# GetMarker/SetMarker store the marker enum's NAME under TextAttribute.UnorderedList —
        // not under TextAttribute.Marker. A C# quirk, ported faithfully.
        [[nodiscard]] marker_type get_marker() const;
        void set_marker(marker_type value);
        [[nodiscard]] bool get_strikethrough() const;
        void set_strikethrough(bool value);
        [[nodiscard]] bool get_superscript() const;
        void set_superscript(bool value);
        [[nodiscard]] bool get_subscript() const;
        void set_subscript(bool value);
        // Foreground/background colors travel as their string form (hex), exactly as C#.
        [[nodiscard]] std::optional<std::string> get_foreground_color() const;
        void set_foreground_color(const std::string& value);
        [[nodiscard]] std::optional<std::string> get_background_color() const;
        void set_background_color(const std::string& value);

        friend bool operator==(const text_attributes& a, const text_attributes& b)
        {
            return a.entries_ == b.entries_;
        }
        friend bool operator!=(const text_attributes& a, const text_attributes& b)
        {
            return !(a == b);
        }

    private:
        std::map<text_attribute, std::string> entries_;
    };
} // namespace maui::graphics::text
