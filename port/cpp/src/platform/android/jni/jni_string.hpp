#pragma once
// maui::platform::android — UTF-8 ⇄ jstring. Internal seam infrastructure for the Android backend,
// NOT a ported MAUI type. The port's cross-platform string type is UTF-8 std::string; java.lang
// .String is UTF-16. JNI's own NewStringUTF/GetStringUTFChars speak MODIFIED UTF-8 (CESU-8 surrogate
// pairs + 2-byte NUL), which silently corrupts supplementary-plane text (emoji) — so these helpers
// go through the real UTF-16 APIs (NewString/GetStringRegion) with explicit UTF-8 ⇄ UTF-16
// transcoding. The transcoders are pure functions (unit-tested on-device without a VM, see
// tests/platform/jni_string_tests.cpp); invalid input maps to U+FFFD like .NET's UTF8Encoding —
// with one documented simplification: a malformed UTF-8 sequence yields one U+FFFD per rejected
// BYTE (resynchronizing byte-by-byte) rather than per maximal subpart.

#include <jni.h>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "jni_ref.hpp"

namespace maui::platform::android
{
    [[nodiscard]] inline std::u16string utf8_to_utf16(std::string_view utf8)
    {
        constexpr char32_t replacement = 0xFFFD;
        std::u16string out;
        out.reserve(utf8.size());
        std::size_t i = 0;
        while (i < utf8.size())
        {
            const auto lead = static_cast<unsigned char>(utf8[i]);
            std::size_t length = 0;
            char32_t code_point = 0;
            if (lead < 0x80U)
            {
                length = 1;
                code_point = lead;
            }
            else if ((lead & 0xE0U) == 0xC0U)
            {
                length = 2;
                code_point = lead & 0x1FU;
            }
            else if ((lead & 0xF0U) == 0xE0U)
            {
                length = 3;
                code_point = lead & 0x0FU;
            }
            else if ((lead & 0xF8U) == 0xF0U)
            {
                length = 4;
                code_point = lead & 0x07U;
            }
            bool valid = length != 0 && i + length <= utf8.size();
            for (std::size_t k = 1; valid && k < length; ++k)
            {
                const auto continuation = static_cast<unsigned char>(utf8[i + k]);
                valid = (continuation & 0xC0U) == 0x80U;
                code_point = (code_point << 6U) | (continuation & 0x3FU);
            }
            if (valid)
            {
                // Reject overlong encodings, UTF-16 surrogate code points, and > U+10FFFF.
                constexpr std::array<char32_t, 5> first_for_length{0, 0, 0x80, 0x800, 0x10000};
                valid = code_point >= first_for_length[length] && code_point <= 0x10FFFF &&
                        (code_point < 0xD800 || code_point > 0xDFFF);
            }
            if (!valid)
            {
                out.push_back(static_cast<char16_t>(replacement));
                ++i; // resynchronize one byte at a time (see the header note)
                continue;
            }
            if (code_point >= 0x10000)
            {
                const char32_t offset = code_point - 0x10000;
                out.push_back(static_cast<char16_t>(0xD800U + (offset >> 10U)));
                out.push_back(static_cast<char16_t>(0xDC00U + (offset & 0x3FFU)));
            }
            else
            {
                out.push_back(static_cast<char16_t>(code_point));
            }
            i += length;
        }
        return out;
    }

    [[nodiscard]] inline std::string utf16_to_utf8(std::u16string_view utf16)
    {
        constexpr char32_t replacement = 0xFFFD;
        std::string out;
        out.reserve(utf16.size() * 3);
        std::size_t i = 0;
        while (i < utf16.size())
        {
            const char16_t unit = utf16[i];
            char32_t code_point = unit;
            if (unit >= 0xD800 && unit <= 0xDBFF)
            {
                if (i + 1 < utf16.size() && utf16[i + 1] >= 0xDC00 && utf16[i + 1] <= 0xDFFF)
                {
                    code_point = 0x10000U + ((static_cast<char32_t>(unit) - 0xD800U) << 10U) +
                                 (static_cast<char32_t>(utf16[i + 1]) - 0xDC00U);
                    ++i; // consumed the trail unit too
                }
                else
                {
                    code_point = replacement; // dangling high surrogate
                }
            }
            else if (unit >= 0xDC00 && unit <= 0xDFFF)
            {
                code_point = replacement; // orphaned low surrogate
            }
            if (code_point < 0x80U)
            {
                out.push_back(static_cast<char>(code_point));
            }
            else if (code_point < 0x800U)
            {
                out.push_back(static_cast<char>(0xC0U | (code_point >> 6U)));
                out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
            }
            else if (code_point < 0x10000U)
            {
                out.push_back(static_cast<char>(0xE0U | (code_point >> 12U)));
                out.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
                out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
            }
            else
            {
                out.push_back(static_cast<char>(0xF0U | (code_point >> 18U)));
                out.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3FU)));
                out.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
                out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
            }
            ++i;
        }
        return out;
    }

    // UTF-8 -> java.lang.String (a new local reference; empty input -> the empty string, never null).
    [[nodiscard]] inline local_ref<jstring> to_jstring(JNIEnv* env, std::string_view utf8)
    {
        const std::u16string utf16 = utf8_to_utf16(utf8);
        // jchar (uint16_t) and char16_t are distinct types; widen via the container instead of a cast.
        const std::vector<jchar> units(utf16.begin(), utf16.end());
        static constexpr jchar no_units = 0; // empty vector data() may be null; CheckJNI dislikes that
        return {env, env->NewString(units.empty() ? &no_units : units.data(), static_cast<jsize>(units.size()))};
    }

    // java.lang.String -> UTF-8 (nullptr -> empty string).
    [[nodiscard]] inline std::string to_utf8(JNIEnv* env, jstring value)
    {
        if (value == nullptr)
        {
            return {};
        }
        const jsize length = env->GetStringLength(value);
        std::vector<jchar> units(static_cast<std::size_t>(length));
        if (length > 0)
        {
            env->GetStringRegion(value, 0, length, units.data());
        }
        return utf16_to_utf8(std::u16string(units.begin(), units.end()));
    }
} // namespace maui::platform::android
