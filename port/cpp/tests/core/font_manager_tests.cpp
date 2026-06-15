// font_manager / font_registrar / font_file tests (headless, cross-platform). These cover the
// platform-agnostic resolution logic: FontFile.FromString parsing, the registrar alias→filename mapping +
// lookup cache, the manager's CleanseFontName chain (alias → file-name variants → PostScript name), the
// font cache (same font → same handle, different font → different handle), and the size-resolution rules.
// The native UIFont/NSFont resolution is covered on-device by font_manager_{apple,ios}_tests.mm; here the
// headless partial returns owned sentinel handles so the cross-platform cache/cleanse logic is asserted
// deterministically. Ported in spirit from FontManagerTests.iOS.cs + the FontFile/FontRegistrar behavior
// in src/Core/src/Fonts.
#include "maui/core/font_manager.hpp"

#include <string>

#include "maui/core/font.hpp"
#include "maui/core/font_file.hpp"
#include "maui/core/font_registrar.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::font;
    using maui::core::font_file;
    using maui::core::font_manager;
    using maui::core::font_registrar;

    // ---- font_file (FontFile.FromString) ----

    TEST(font_file, bare_family_is_its_own_filename_and_postscript_name)
    {
        const font_file parsed = font_file::from_string("Arial");
        EXPECT_EQ(parsed.file_name(), "Arial");
        EXPECT_TRUE(parsed.extension().empty());
        EXPECT_EQ(parsed.post_script_name(), "Arial");
    }

    TEST(font_file, peels_a_trailing_supported_extension)
    {
        const font_file ttf = font_file::from_string("Lobster.ttf");
        EXPECT_EQ(ttf.file_name(), "Lobster");
        EXPECT_EQ(ttf.extension(), ".ttf");
        EXPECT_EQ(ttf.post_script_name(), "Lobster.ttf"); // no '#' → the whole input is the PostScript name

        // Case-insensitive match, but the STORED extension is the canonical lowercase from Extensions
        // (C# FontFile.FromString assigns `foundExtension = extension`, the table entry, not the input's case).
        const font_file otf = font_file::from_string("Lobster.OTF");
        EXPECT_EQ(otf.file_name(), "Lobster");
        EXPECT_EQ(otf.extension(), ".otf");
    }

    TEST(font_file, hash_splits_family_and_strips_postscript_spaces)
    {
        // "CuteFont-Regular#Cute Font" → family "CuteFont-Regular", PostScript "CuteFont" (spaces stripped).
        const font_file parsed = font_file::from_string("CuteFont-Regular#Cute Font");
        EXPECT_EQ(parsed.file_name(), "CuteFont-Regular");
        EXPECT_EQ(parsed.post_script_name(), "CuteFont");
    }

    TEST(font_file, file_name_with_extension)
    {
        const font_file parsed = font_file::from_string("Lobster");
        EXPECT_EQ(parsed.file_name_with_extension(".ttf"), "Lobster.ttf");
    }

    // ---- font_registrar ----

    TEST(font_registrar, registers_filename_and_alias)
    {
        font_registrar registrar;
        registrar.register_font("Lobster.ttf", "Lobster");
        EXPECT_EQ(registrar.get_font("Lobster.ttf"), "Lobster.ttf");
        EXPECT_EQ(registrar.get_font("Lobster"), "Lobster.ttf"); // the alias resolves to the filename
    }

    TEST(font_registrar, empty_alias_registers_only_the_filename)
    {
        font_registrar registrar;
        registrar.register_font("Lobster.ttf", "");
        EXPECT_EQ(registrar.get_font("Lobster.ttf"), "Lobster.ttf");
        EXPECT_TRUE(registrar.get_font("Lobster").empty()); // no alias registered
    }

    TEST(font_registrar, unregistered_font_returns_empty)
    {
        font_registrar registrar;
        EXPECT_TRUE(registrar.get_font("DoesNotExist").empty());
        // A repeat lookup hits the cached miss — still empty.
        EXPECT_TRUE(registrar.get_font("DoesNotExist").empty());
    }

    // ---- font_manager (cross-platform CleanseFontName + cache + size resolution) ----

    TEST(font_manager, cleanse_resolves_an_alias_to_its_filename)
    {
        font_registrar registrar;
        registrar.register_font("Lobster.ttf", "Lobster");
        const font_manager manager(registrar);
        // CleanseFontName tries the alias first (registrar.GetFont) → the filename.
        EXPECT_EQ(manager.cleanse_font_name("Lobster"), "Lobster.ttf");
    }

    TEST(font_manager, cleanse_probes_extension_variants_for_a_bare_name)
    {
        font_registrar registrar;
        registrar.register_font("Lobster.ttf", ""); // registered under the WITH-extension filename only
        const font_manager manager(registrar);
        // A bare "Lobster" misses the alias, then probes "Lobster.ttf"/"Lobster.otf" → the .ttf hit.
        EXPECT_EQ(manager.cleanse_font_name("Lobster"), "Lobster.ttf");
    }

    TEST(font_manager, cleanse_falls_back_to_the_post_script_name)
    {
        font_registrar registrar; // nothing registered
        const font_manager manager(registrar);
        // No registration → the parsed PostScript name (the input, here with no '#').
        EXPECT_EQ(manager.cleanse_font_name("Unregistered"), "Unregistered");
        // With a '#', the PostScript half (spaces stripped) is the fallback.
        EXPECT_EQ(manager.cleanse_font_name("Family#Post Script"), "PostScript");
    }

    TEST(font_manager, get_font_caches_the_same_font)
    {
        font_manager manager; // process-wide registrar
        const void* const a = manager.get_font(font::of_size("", 20));
        const void* const b = manager.get_font(font::of_size("", 20));
        EXPECT_NE(a, nullptr);
        EXPECT_EQ(a, b); // C# _fonts.GetOrAdd — the same font hits the cache
    }

    TEST(font_manager, get_font_distinguishes_different_fonts)
    {
        font_manager manager;
        const void* const a = manager.get_font(font::of_size("", 20));
        const void* const b = manager.get_font(font::of_size("", 24)); // different size → different cache entry
        EXPECT_NE(a, nullptr);
        EXPECT_NE(b, nullptr);
        EXPECT_NE(a, b);
    }

    TEST(font_manager, get_font_folds_the_default_size_for_a_sizeless_font)
    {
        font_manager manager;
        // A sizeless font resolved with default 20 must equal the same font resolved at an explicit 20 (the
        // size is folded onto the key, so both land on one cache entry — C# GetFont's WithSize step).
        const void* const folded = manager.get_font(font::of_size("", 0), 20.0);
        const void* const explicit_size = manager.get_font(font::of_size("", 20.0));
        EXPECT_EQ(folded, explicit_size);
    }

    TEST(font_manager, default_font_is_non_null_and_stable)
    {
        font_manager manager;
        const void* const a = manager.default_font();
        const void* const b = manager.default_font();
        EXPECT_NE(a, nullptr);
        EXPECT_EQ(a, b); // lazily created once, reused (C# _defaultFont)
    }
} // namespace
