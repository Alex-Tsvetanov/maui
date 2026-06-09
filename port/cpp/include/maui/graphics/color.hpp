#pragma once
// maui::graphics::color  <=  Microsoft.Maui.Graphics.Color
//
// Immutable RGBA color, float components in [0,1]. Ported from
// src/Graphics/src/Graphics/Color.cs (+ ColorUtils.cs for parsing & HSL/HSV math,
// Colors.cs for the named-color table). The C# original is an immutable reference
// type; here it is a value type. Equality compares the truncated ARGB int
// (to_int), exactly matching C# Color.Equals.
//
// System.Numerics interop: the maui::graphics::vector4 stand-in mirrors C#'s Color(Vector4) ctor +
// `implicit operator Color(Vector4)` (RGBA = X/Y/Z/W, each clamped to [0,1]). to_vector4() is the
// reverse (RGBA, unclamped — the stored components are already in range); C# has no such member, so
// it is a port convenience kept consistent with the ctor's field mapping.
//
// Deliberate M0 deviation (recorded in port/STATUS.md):
//  - C# byte/int CONSTRUCTORS are exposed only as the from_rgb / from_rgba
//    factories: the integer ctors would be ambiguous with the float ctors under
//    C++'s implicit numeric conversions (C# forbids implicit float->int, C++ does not).

#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>

namespace maui::graphics
{
    struct vector4;

    class color
    {
    public:
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;
        float alpha = 1.0f;

        constexpr color() = default; // black, alpha = 1
        explicit color(float gray);
        color(float r, float g, float b);
        color(float r, float g, float b, float a);
        color(const vector4& v); // implicit Vector4 -> Color (RGBA, clamped); matches C#

        // ---- factories: INTEGER components are 0-255 (divided by 255) ----
        static color from_rgb(int r, int g, int b);
        static color from_rgba(int r, int g, int b, int a);

        // ---- factories: FLOATING components are 0-1 (used directly, clamped) ----
        // Selected over the integer overload whenever any argument is floating,
        // mirroring C#'s overload set (where float->int is not an implicit conversion).
        template <class R, class G, class B>
            requires(std::floating_point<R> || std::floating_point<G> || std::floating_point<B>)
        static color from_rgb(R r, G g, B b)
        {
            return color(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
        }
        template <class R, class G, class B, class A>
            requires(std::floating_point<R> || std::floating_point<G> || std::floating_point<B> ||
                     std::floating_point<A>)
        static color from_rgba(R r, G g, B b, A a)
        {
            return color(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a));
        }

        static color from_int(int argb);            // 0xAARRGGBB
        static color from_uint(std::uint32_t argb); // 0xAARRGGBB

        // ---- hex strings ----
        static color from_argb(std::string_view color_as_hex); // #aarrggbb / #rrggbb / #argb / #rgb
        static color from_rgba(std::string_view color_as_hex); // #rrggbbaa / #rrggbb / #rgba / #rgb
        [[deprecated("use from_argb")]] static color from_hex(std::string_view color_as_argb_hex);

        // ---- HSL / HSV ----
        static color from_hsla(float h, float s, float l, float a = 1.0f);    // all 0-1
        static color from_hsla(double h, double s, double l, double a = 1.0); // all 0-1
        static color from_hsv(int h, int s, int v);                           // h:0-360, s,v:0-100
        static color from_hsva(int h, int s, int v, int a);                   // h:0-360, s,v,a:0-100
        template <class H, class S, class V>
            requires(std::floating_point<H> || std::floating_point<S> || std::floating_point<V>)
        static color from_hsv(H h, S s, V v)
        { // all 0-1, direct
            return from_hsva_unit(static_cast<float>(h), static_cast<float>(s), static_cast<float>(v), 1.0f);
        }
        template <class H, class S, class V, class A>
            requires(std::floating_point<H> || std::floating_point<S> || std::floating_point<V> ||
                     std::floating_point<A>)
        static color from_hsva(H h, S s, V v, A a)
        { // all 0-1, direct
            return from_hsva_unit(static_cast<float>(h), static_cast<float>(s), static_cast<float>(v),
                                  static_cast<float>(a));
        }

        // ---- parse (hex, rgb()/rgba(), hsl()/hsla(), hsv()/hsva(), named) ----
        static color parse(std::string_view value);
        static bool try_parse(std::string_view value, color& out);

        // ---- conversions ----
        int to_int() const; // 0xAARRGGBB (truncated bytes)
        std::uint32_t to_uint() const;
        void to_rgb(std::uint8_t& r, std::uint8_t& g, std::uint8_t& b) const;
        void to_rgba(std::uint8_t& r, std::uint8_t& g, std::uint8_t& b, std::uint8_t& a) const;
        vector4 to_vector4() const; // RGBA -> Vector4 (port convenience; see header note)

        // ---- hex out ----
        std::string to_hex() const; // #rrggbb
        std::string to_argb_hex(bool include_alpha = false) const;
        std::string to_rgba_hex(bool include_alpha = false) const;

        // ---- HSL accessors / modifiers ----
        void to_hsl(float& h, float& s, float& l) const;
        float get_hue() const;
        float get_saturation() const;
        float get_luminosity() const;
        color with_luminosity(float luminosity) const;
        color add_luminosity(float delta) const;
        color with_hue(float hue) const;
        color with_saturation(float saturation) const;
        color get_complementary() const;

        // ---- alpha ----
        color with_alpha(float a) const;
        color multiply_alpha(float multiply_by) const;

        std::string to_string() const;

        friend bool operator==(const color& a, const color& b)
        {
            return a.to_int() == b.to_int();
        }
        friend bool operator!=(const color& a, const color& b)
        {
            return !(a == b);
        }

    private:
        static color from_hsva_unit(float h, float s, float v, float a); // ConvertHsvToRgb
    };

} // namespace maui::graphics

// std::hash kept consistent with operator== (both derive from the ARGB int).
template <> struct std::hash<maui::graphics::color>
{
    std::size_t operator()(const maui::graphics::color& c) const noexcept
    {
        return std::hash<std::uint32_t>{}(c.to_uint());
    }
};
