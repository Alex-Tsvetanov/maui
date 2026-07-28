// winui_interop — the string/color conversions declared in winui_interop.hpp. The void*-slot helpers
// are templates and live in the header; only these three need a translation unit.

#include "winui_interop.hpp"

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
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
    } // namespace

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
} // namespace maui::platform::windows
