// winui_interop — the string/color conversions declared in winui_interop.hpp. The void*-slot helpers
// are templates and live in the header; only these three need a translation unit.

#include "winui_interop.hpp"

// IMap members (HasKey/Lookup below, on Application::Current().Resources()) — the C++/WinRT include
// rule (see winui_interop.hpp's file header): the impl/*.0.h headers reached transitively only
// forward-declare those, and calling them without this full header fails with C3779, an error that
// does not read as "add an include".
#include <winrt/Windows.Foundation.Collections.h>

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace maui::platform::windows
{
    namespace
    {
        // TRUNCATE, do not round. ColorExtensions.ToWindowsColor is
        //     Color.FromArgb((byte)(color.Alpha * 255), (byte)(color.Red * 255), ...)
        // and a C# (byte) cast on a float truncates toward zero. Rounding is the more "correct" quantizer
        // and produces a DIFFERENT byte for most non-terminal values (0.5f -> 128 rounded, 127
        // truncated), which would put the port one level off MAUI on every mid-tone the board compares.
        // Parity ruling 1: MAUI's actual render is the ground truth, including where it is imprecise.
        std::uint8_t to_byte(float component)
        {
            return static_cast<std::uint8_t>(std::clamp(component, 0.0F, 1.0F) * 255.0F);
        }

        // The pre-lookup values every handler hard-coded before default_font_family()/default_font_size()
        // existed (see winui_interop.hpp's doc comment) — the WinUI theme resources they now resolve carry
        // "Segoe UI Variable Text" / 14 on Windows 11. Kept as the fallback for a missing/mistyped key.
        constexpr double k_default_font_size = 14.0;
        constexpr std::wstring_view k_default_font_family = L"Segoe UI Variable Text";
    } // namespace

    float measure_constraint(double value)
    {
        if (std::isnan(value))
        {
            return std::numeric_limits<float>::infinity();
        }
        return static_cast<float>(value);
    }

    winrt::hstring to_hstring(std::string_view utf8)
    {
        if (utf8.empty())
        {
            return {};
        }
        // Two-pass MultiByteToWideChar: ask for the length, then fill. Sizing the buffer by utf8.size()
        // instead would be correct only for ASCII and would silently truncate multi-byte text.
        const int needed = ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
        if (needed <= 0)
        {
            return {};
        }
        std::wstring wide(static_cast<std::size_t>(needed), L'\0');
        ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), wide.data(), needed);
        return winrt::hstring{wide};
    }

    std::string to_utf8(const winrt::hstring& text)
    {
        const std::wstring_view wide{text};
        if (wide.empty())
        {
            return {};
        }
        const int needed =
            ::WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
        if (needed <= 0)
        {
            return {};
        }
        std::string out(static_cast<std::size_t>(needed), '\0');
        ::WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), out.data(), needed, nullptr,
                              nullptr);
        return out;
    }

    winrt::Windows::UI::Color to_ui_color(const maui::graphics::color& value)
    {
        return winrt::Windows::UI::Color{to_byte(value.alpha), to_byte(value.red), to_byte(value.green),
                                         to_byte(value.blue)};
    }

    // FontManager.Windows.cs:49-56 — `_defaultFontFamily ??= (FontFamily)Application.Current
    // .Resources["ContentControlThemeFontFamily"]`. Resolved once and cached (function-local static
    // matches the oracle's `??=`), falling back to the pre-lookup constant when the key is absent or not
    // actually a FontFamily, rather than throwing like the oracle's direct indexer/cast would.
    winrt::Microsoft::UI::Xaml::Media::FontFamily default_font_family()
    {
        static const winrt::Microsoft::UI::Xaml::Media::FontFamily resolved = [] {
            const auto resources = winrt::Microsoft::UI::Xaml::Application::Current().Resources();
            const auto key = winrt::box_value(winrt::hstring{L"ContentControlThemeFontFamily"});
            if (resources.HasKey(key))
            {
                if (const auto family = resources.Lookup(key).try_as<winrt::Microsoft::UI::Xaml::Media::FontFamily>())
                {
                    return family;
                }
            }
            return winrt::Microsoft::UI::Xaml::Media::FontFamily{k_default_font_family};
        }();
        return resolved;
    }

    // FontManager.Windows.cs:59-66 — `_defaultFontSize ??= (double)Application.Current
    // .Resources["ControlContentThemeFontSize"]`. Same resolve-once-and-cache / same-key-class shape as
    // DefaultFontFamily above (the oracle defines both members identically); same throws-vs-degrades
    // reasoning for the fallback.
    double default_font_size()
    {
        static const double resolved = [] {
            const auto resources = winrt::Microsoft::UI::Xaml::Application::Current().Resources();
            const auto key = winrt::box_value(winrt::hstring{L"ControlContentThemeFontSize"});
            if (resources.HasKey(key))
            {
                return winrt::unbox_value_or<double>(resources.Lookup(key), k_default_font_size);
            }
            return k_default_font_size;
        }();
        return resolved;
    }
} // namespace maui::platform::windows
