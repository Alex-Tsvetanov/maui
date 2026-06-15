// iOS (UIKit) tests for font_manager: it resolves a maui::core::font to a real UIFont. The faithful twin
// of font_manager_apple_tests.mm. A known installed family ("Helvetica") resolves to a non-null UIFont of
// that family at the requested size; an unknown family falls back to the system font (never null); the
// sizeless font uses UIFont.systemFontSize; bold weight carries a Bold trait; the cache returns the same
// UIFont. Mirrors FontManagerTests.iOS.CanLoadSystemFonts. Run on the simulator. Compiled as Obj-C++ ARC.
#import <UIKit/UIKit.h>

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

    UIFont* as_font(void* handle)
    {
        return (__bridge UIFont*)handle;
    }

    TEST(ios_font_manager, resolves_a_known_family_to_a_non_null_native_font)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        UIFont* const resolved = as_font(manager.get_font(font::of_size("Helvetica", 18)));
        ASSERT_NE(resolved, nil);
        EXPECT_EQ(resolved.pointSize, 18.0);
        EXPECT_TRUE([resolved.familyName isEqualToString:@"Helvetica"]);
    }

    TEST(ios_font_manager, unknown_family_falls_back_to_the_system_font)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        UIFont* const resolved = as_font(manager.get_font(font::of_size("NoSuchFamilyXYZ", 14)));
        ASSERT_NE(resolved, nil); // never null — the system font is the floor
        EXPECT_EQ(resolved.pointSize, 14.0);
    }

    TEST(ios_font_manager, sizeless_font_uses_the_default_font_size)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        UIFont* const resolved = as_font(manager.get_font(font::system_font_of_size(0)));
        ASSERT_NE(resolved, nil);
        EXPECT_EQ(resolved.pointSize, UIFont.systemFontSize);
    }

    TEST(ios_font_manager, bold_weight_yields_a_bold_trait)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        UIFont* const bold =
            as_font(manager.get_font(font::system_font_of_size(16, font_weight::bold, font_slant::normal)));
        ASSERT_NE(bold, nil);
        const UIFontDescriptorSymbolicTraits traits = bold.fontDescriptor.symbolicTraits;
        EXPECT_TRUE((traits & UIFontDescriptorTraitBold) != 0);
    }

    TEST(ios_font_manager, caches_the_same_font)
    {
        font_registrar registrar;
        font_manager manager(registrar);
        void* const a = manager.get_font(font::of_size("Helvetica", 18));
        void* const b = manager.get_font(font::of_size("Helvetica", 18));
        EXPECT_EQ(a, b); // the same font hits the cache (C# _fonts.GetOrAdd)
    }

    TEST(ios_font_manager, default_font_size_is_the_system_font_size)
    {
        font_manager manager;
        EXPECT_EQ(manager.default_font_size(), static_cast<double>(UIFont.systemFontSize));
    }
} // namespace
