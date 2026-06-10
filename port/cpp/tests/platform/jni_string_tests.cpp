// UTF-8 ⇄ UTF-16 transcoding tests for the Android JNI seam (src/platform/android/jni/
// jni_string.hpp). The transcoders are pure functions, so these run as ordinary on-device gtest
// cases without a Java VM; the jstring round trip itself is exercised by the app_process widget
// probe (testhost/jni_probe.cpp). Characterization oracle: .NET's UTF8Encoding behavior (invalid
// input -> U+FFFD), with the documented one-replacement-per-rejected-byte simplification.

#include "jni/jni_string.hpp"

#include <string>

#include <gtest/gtest.h>

namespace
{
    using maui::platform::android::utf16_to_utf8;
    using maui::platform::android::utf8_to_utf16;

    TEST(jni_string, ascii_round_trips)
    {
        const std::string utf8 = "hello maui";
        const std::u16string utf16 = utf8_to_utf16(utf8);
        EXPECT_EQ(utf16, u"hello maui");
        EXPECT_EQ(utf16_to_utf8(utf16), utf8);
    }

    TEST(jni_string, empty_round_trips)
    {
        EXPECT_TRUE(utf8_to_utf16("").empty());
        EXPECT_TRUE(utf16_to_utf8(u"").empty());
    }

    TEST(jni_string, bmp_text_round_trips)
    {
        // 2-byte (é, ü), 3-byte (€, CJK, kana) sequences.
        const std::string utf8 = "héllo ünïcode € 漢字 かな";
        const std::u16string utf16 = utf8_to_utf16(utf8);
        EXPECT_EQ(utf16, u"héllo ünïcode € 漢字 かな");
        EXPECT_EQ(utf16_to_utf8(utf16), utf8);
    }

    TEST(jni_string, supplementary_plane_uses_surrogate_pairs)
    {
        // U+1F9E9 (🧩) — the modified-UTF-8 trap NewStringUTF would mangle; must become D83E DDE9.
        const std::string utf8 = "\xF0\x9F\xA7\xA9";
        const std::u16string utf16 = utf8_to_utf16(utf8);
        ASSERT_EQ(utf16.size(), 2U);
        EXPECT_EQ(utf16[0], char16_t{0xD83E});
        EXPECT_EQ(utf16[1], char16_t{0xDDE9});
        EXPECT_EQ(utf16_to_utf8(utf16), utf8);
    }

    TEST(jni_string, invalid_utf8_becomes_replacement_chars)
    {
        // A stray continuation byte, a truncated 3-byte lead, and a bare 0xFF: each rejected byte
        // maps to one U+FFFD (the documented per-byte resynchronization rule).
        EXPECT_EQ(utf8_to_utf16("\x80"), u"�");
        EXPECT_EQ(utf8_to_utf16("\xE2\x82"), u"��"); // truncated '€'
        EXPECT_EQ(utf8_to_utf16("a\xFF"
                                "b"),
                  u"a�"
                  "b");
    }

    TEST(jni_string, overlong_and_surrogate_utf8_rejected)
    {
        // Overlong NUL (C0 80 — exactly what modified UTF-8 would emit) and an encoded UTF-16
        // surrogate (ED A0 80 = U+D800) are invalid real UTF-8.
        EXPECT_EQ(utf8_to_utf16("\xC0\x80"), u"��");
        EXPECT_EQ(utf8_to_utf16("\xED\xA0\x80"), u"���");
    }

    TEST(jni_string, lone_surrogates_in_utf16_become_replacement_chars)
    {
        EXPECT_EQ(utf16_to_utf8(u"a\xD83E"), "a\xEF\xBF\xBD");                 // dangling high surrogate
        EXPECT_EQ(utf16_to_utf8(u"\xDDE9z"), "\xEF\xBF\xBDz");                 // orphaned low surrogate
        EXPECT_EQ(utf16_to_utf8(u"\xD83E\xD83E"), "\xEF\xBF\xBD\xEF\xBF\xBD"); // high+high
    }
} // namespace
