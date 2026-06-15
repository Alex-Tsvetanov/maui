// shell_appearance_tests — the W7-51 Shell appearance model: the shell_appearance value type
// (Ingest precedence, effective tab-bar fallback, equality), the Shell.* color ATTACHED properties, and
// the Shell.GetAppearanceForPivot resolution walk (down to the current page, then up the hierarchy,
// lowest-level-wins per slot, plus the root-content-under-pushed-page deviation). Ported from
// ShellAppearanceTests.cs (ColorSetCorrectly) + the GetAppearanceForPivot / Ingest source as the oracle.
//
// §8: fixture-owned pages outlive every test-body local; shells are declared before any subscriber.

#include <optional>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_appearance.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/graphics/colors.hpp"
#include "tests/controls/shell_test_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using namespace maui::controls;              // shell, shell_item, shell_appearance, …
    using namespace maui::controls::shell_tests; // shell_test_base
    namespace colors = maui::graphics::colors;

    using shell_appearance_test = shell_test_base;

    // ---- the value type: Ingest + effective fallback + equality ----

    // C# ShellAppearanceTests.ColorSetCorrectly: set Shell.DisabledColor on an item, Ingest it, read it back.
    TEST_F(shell_appearance_test, color_set_correctly)
    {
        auto item = create_shell_item<flyout_item>();
        shell::set_disabled_color(*item, colors::purple);

        shell_appearance result;
        result.ingest(*item);

        // Compare whole optionals (avoids an unchecked .value() after the has_value() guard).
        EXPECT_EQ(result.disabled_color(), std::optional{colors::purple});
    }

    // Ingest fills every kind of slot from one element (the ten colors + the two flyout doubles).
    TEST_F(shell_appearance_test, ingest_fills_all_slot_kinds)
    {
        auto item = create_shell_item<flyout_item>();
        shell::set_background_color(*item, colors::red);
        shell::set_tab_bar_title_color(*item, colors::green);
        shell::set_flyout_width(*item, 320.0);
        shell::set_flyout_height(*item, 480.0);

        shell_appearance result;
        EXPECT_TRUE(result.ingest(*item));
        EXPECT_EQ(result.background_color(), std::optional{colors::red});
        EXPECT_EQ(result.tab_bar_title_color(), std::optional{colors::green});
        EXPECT_EQ(result.flyout_width(), std::optional{320.0});
        EXPECT_EQ(result.flyout_height(), std::optional{480.0});
    }

    // Ingest returns false (and changes nothing) when the element set no appearance values.
    TEST_F(shell_appearance_test, ingest_returns_false_when_nothing_set)
    {
        auto item = create_shell_item<flyout_item>();
        shell_appearance result;
        EXPECT_FALSE(result.ingest(*item));
        EXPECT_FALSE(result.background_color().has_value());
    }

    // The C# Ingest precedence: a slot already SET in the appearance is NOT overwritten by a later ingest
    // (so the first/lower element to provide a value wins). The first ingest takes red; the second is ignored.
    TEST_F(shell_appearance_test, ingest_does_not_overwrite_a_set_slot)
    {
        auto first = create_shell_item<flyout_item>();
        auto second = create_shell_item<flyout_item>();
        shell::set_background_color(*first, colors::red);
        shell::set_background_color(*second, colors::blue);

        shell_appearance result;
        EXPECT_TRUE(result.ingest(*first));   // red wins (set first)
        EXPECT_FALSE(result.ingest(*second)); // blue is ignored — slot already set, nothing new ingested
        EXPECT_EQ(result.background_color(), std::optional{colors::red});
    }

    // IShellAppearanceElement: each effective tab-bar color is the tab-bar slot when set, else the shell-wide
    // counterpart (TabBarBackgroundColor ?? BackgroundColor, etc.).
    TEST_F(shell_appearance_test, effective_tab_bar_falls_back_to_base)
    {
        auto item = create_shell_item<flyout_item>();
        shell::set_background_color(*item, colors::red);           // base only
        shell::set_tab_bar_foreground_color(*item, colors::green); // tab-bar slot set

        shell_appearance result;
        result.ingest(*item);

        // background: tab-bar slot unset → falls back to base background.
        EXPECT_EQ(result.effective_tab_bar_background_color(), std::optional{colors::red});
        // foreground: tab-bar slot set → uses it, not the (unset) base foreground.
        EXPECT_EQ(result.effective_tab_bar_foreground_color(), std::optional{colors::green});
        // title: neither set → nullopt.
        EXPECT_EQ(result.effective_tab_bar_title_color(), std::nullopt);
    }

    // Equality compares every color slot + both doubles (ShellAppearance.Equals / operator==).
    TEST_F(shell_appearance_test, equality_compares_all_slots)
    {
        auto a_item = create_shell_item<flyout_item>();
        auto b_item = create_shell_item<flyout_item>();
        auto c_item = create_shell_item<flyout_item>();
        shell::set_background_color(*a_item, colors::red);
        shell::set_background_color(*b_item, colors::red);
        shell::set_background_color(*c_item, colors::blue);

        shell_appearance a;
        shell_appearance b;
        shell_appearance c;
        a.ingest(*a_item);
        b.ingest(*b_item);
        c.ingest(*c_item);

        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a == c);
        EXPECT_TRUE(shell_appearance{} == shell_appearance{}); // two empties are equal
    }

    // ---- the attached-property accessors round-trip ----

    TEST_F(shell_appearance_test, attached_color_getter_round_trips)
    {
        auto page = make_page();
        EXPECT_EQ(shell::get_title_color(*page), std::nullopt); // unset
        shell::set_title_color(*page, colors::orange);
        EXPECT_EQ(shell::get_title_color(*page), std::optional{colors::orange});
        // Independent elements don't share the attached value.
        auto other = make_page();
        EXPECT_EQ(shell::get_title_color(*other), std::nullopt);
    }

    // ---- the resolution walk (Shell.GetAppearanceForPivot) ----

    // A color set on a SHELL ITEM resolves for the current page (walk down to the page, then up to the item).
    TEST_F(shell_appearance_test, resolve_picks_up_item_color)
    {
        shell sh;
        auto item = create_shell_item<flyout_item>();
        shell::set_background_color(*item, colors::red);
        sh.add_item(item);

        content_page* const page = sh.current_page();
        ASSERT_NE(page, nullptr);

        const std::optional<shell_appearance> resolved = shell::get_appearance_for_pivot(*page);
        ASSERT_TRUE(resolved.has_value());
        // value_or with an empty default is the unchecked-access-clean read (the ASSERT above already
        // guaranteed presence, so the default is never taken).
        EXPECT_EQ(resolved.value_or(shell_appearance{}).background_color(), std::optional{colors::red});
    }

    // Nothing set anywhere in the line → the resolve returns nullopt (the chrome keeps its defaults).
    TEST_F(shell_appearance_test, resolve_returns_nullopt_when_nothing_set)
    {
        shell sh;
        sh.add_item(create_shell_item<flyout_item>());
        content_page* const page = sh.current_page();
        ASSERT_NE(page, nullptr);
        EXPECT_EQ(shell::get_appearance_for_pivot(*page), std::nullopt);
    }

    // Lowest-in-the-tree wins: a color set on BOTH the content page (lowest) and the shell item (highest)
    // resolves to the page's value (the page is ingested first as the walk moves up).
    TEST_F(shell_appearance_test, resolve_lowest_level_wins)
    {
        shell sh;
        auto page = make_page();
        auto item = create_shell_item<flyout_item>(page);
        shell::set_background_color(*item, colors::red);  // higher in the tree
        shell::set_background_color(*page, colors::blue); // lowest — should win
        sh.add_item(item);

        ASSERT_EQ(page.get(), sh.current_page());
        const std::optional<shell_appearance> resolved = shell::get_appearance_for_pivot(*page);
        ASSERT_TRUE(resolved.has_value());
        EXPECT_EQ(resolved.value_or(shell_appearance{}).background_color(), std::optional{colors::blue});
    }

    // Different slots merge across levels: background from the item, title from the page.
    TEST_F(shell_appearance_test, resolve_merges_slots_across_levels)
    {
        shell sh;
        auto page = make_page();
        auto item = create_shell_item<flyout_item>(page);
        shell::set_background_color(*item, colors::red);
        shell::set_title_color(*page, colors::green);
        sh.add_item(item);

        const std::optional<shell_appearance> resolved = shell::get_appearance_for_pivot(*page);
        ASSERT_TRUE(resolved.has_value());
        const shell_appearance merged = resolved.value_or(shell_appearance{});
        EXPECT_EQ(merged.background_color(), std::optional{colors::red});
        EXPECT_EQ(merged.title_color(), std::optional{colors::green});
    }
} // namespace
