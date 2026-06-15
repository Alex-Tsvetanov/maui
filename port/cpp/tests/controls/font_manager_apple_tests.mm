// Apple (AppKit) tests for font_manager: it resolves a maui::core::font to a real NSFont. A known installed
// family ("Helvetica") resolves to a non-null NSFont of that family at the requested size; an unknown
// family falls back to the system font (never null); the empty/sizeless font uses the system size; the
// cache returns the same NSFont for the same font. Mirrors FontManagerTests.iOS (CanLoadSystemFonts —
// the .SFUI-* system family resolves to the system font family). Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <string>

#include "maui/core/font.hpp"
#include "maui/core/font_manager.hpp"
#include "maui/core/font_registrar.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::font;
    using maui::core::font_manager;
    using maui::core::font_registrar;
    using maui::core::font_slant;
    using maui::core::font_weight;

    NSFont* as_font(void* handle)
    {
        return (__bridge NSFont*)handle;
    }

    class apple_font_manager : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_font_manager, resolves_a_known_family_to_a_non_null_native_font)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        NSFont* const resolved = as_font(manager.get_font(font::of_size("Helvetica", 18)));
        ASSERT_NE(resolved, nil);
        EXPECT_EQ(resolved.pointSize, 18.0);
        EXPECT_TRUE([resolved.familyName isEqualToString:@"Helvetica"]);
    }

    TEST_F(apple_font_manager, unknown_family_falls_back_to_the_system_font)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        NSFont* const resolved = as_font(manager.get_font(font::of_size("NoSuchFamilyXYZ", 14)));
        ASSERT_NE(resolved, nil); // never null — the system font is the floor
        EXPECT_EQ(resolved.pointSize, 14.0);
    }

    TEST_F(apple_font_manager, sizeless_font_uses_the_default_font_size)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        NSFont* const resolved = as_font(manager.get_font(font::system_font_of_size(0)));
        ASSERT_NE(resolved, nil);
        EXPECT_EQ(resolved.pointSize, NSFont.systemFontSize);
    }

    TEST_F(apple_font_manager, bold_weight_yields_a_bold_trait)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        NSFont* const bold =
            as_font(manager.get_font(font::system_font_of_size(16, font_weight::bold, font_slant::normal)));
        ASSERT_NE(bold, nil);
        const NSFontSymbolicTraits traits = bold.fontDescriptor.symbolicTraits;
        EXPECT_TRUE((traits & NSFontDescriptorTraitBold) != 0);
    }

    TEST_F(apple_font_manager, caches_the_same_font)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        void* const a = manager.get_font(font::of_size("Helvetica", 18));
        void* const b = manager.get_font(font::of_size("Helvetica", 18));
        EXPECT_EQ(a, b); // the same font hits the cache (C# _fonts.GetOrAdd)
    }

    TEST_F(apple_font_manager, default_font_size_is_the_system_font_size)
    {
        font_manager manager;
        EXPECT_EQ(manager.default_font_size(), static_cast<double>(NSFont.systemFontSize));
    }
} // namespace
