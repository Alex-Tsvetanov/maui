// Phase-4 corpus gate: every gallery-page XAML file must hydrate through the runtime loader as a PLAIN
// ContentPage on the supported surface. The corpus is the union of the CANONICAL SHARED pages
// (port/maui-reference/pages/*.xaml — the same bytes real .NET MAUI compiles; SHARED_PAGES_DIR) and the
// not-yet-migrated legacy twins (examples/gallery_xaml/Views/*.xaml; GALLERY_TWINS_DIR), matching what
// port/tools/e2e/e2e.py gen embeds into the gallery_xaml app. Each .xaml file becomes one parametrized
// case: it must load with no thrown xaml_parse_exception and produce a non-null content tree. Dropping a
// new page into either directory adds a case automatically (a shared page supersedes a same-key twin).
//
// Both dirs are absolute source-tree paths, injected by CMake (target_compile_definitions).
//
// P3 gap corpus (maui-reference/docs/AUTHORING.md rule 6): `gap_*.xaml` pages are DELIBERATELY malformed
// from the port's point of view — each pins exactly one unsupported XAML feature / unregistered control,
// verified to fail with a specific expected_port_status in pages/manifest.json (parse_error /
// unregistered_type / degrades). This gate reads that one field per page (a tiny hand-rolled scanner —
// no JSON dependency exists in this repo yet — reads only the flat "key"/"expected_port_status" string
// pairs the manifest actually contains) so a gap page's expected failure does not red this test, while
// flipping the assertion direction: if a gap page unexpectedly LOADS CLEANLY, that is itself a test
// failure ("gap closed — update the manifest"), per AUTHORING.md's stated design.

#include "maui/xaml/xaml_loader.hpp"
#include "maui/xaml/xaml_runtime_bindings.hpp" // register_runtime_bindings (twins with {Binding})
#include "maui/xaml/xaml_static_check.hpp"     // find_inline_event_attribute (the compile-time gate)

#include "maui/controls/content_page.hpp"
#include "maui/core/i_view.hpp"

#include "../support/gallery_twin_corpus.hpp" // twin_files / read_file / load_expected_statuses

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using maui::xaml::xaml_load_result;
    using maui::xaml::xaml_loader;

    // twin_files / read_file / load_expected_statuses: shared with the render-gate TU (was hand-synced).
    using maui::test::load_expected_statuses;
    using maui::test::read_file;
    using maui::test::twin_files;

    class gallery_twin : public testing::TestWithParam<std::filesystem::path>
    {
    };

    TEST_P(gallery_twin, loads_as_a_plain_content_page)
    {
        // Twins may use {Binding} (e.g. an ItemTemplate's {Binding .} cell, items/collectionview); install
        // the real runtime-binding applier so those load instead of hitting the rejecting default. Idempotent
        // (just sets the global applier), so it is safe to re-register per test even if a sibling TU's
        // runtime_bindings_guard reset it.
        maui::xaml::register_runtime_bindings();

        const std::string xaml = read_file(GetParam());
        ASSERT_FALSE(xaml.empty()) << "empty twin file: " << GetParam();

        const std::string key = GetParam().stem().string();
        static const std::map<std::string, std::string> expected_statuses = load_expected_statuses();
        const auto it = expected_statuses.find(key);
        // Gap pages (pages/manifest.json expected_port_status != "renders") are DELIBERATELY expected to
        // fail to load — parse_error / unregistered_type / degrades all mean "does not load cleanly".
        // "degrades" pages DO load (they just render wrong), so only exempt the two load-failure statuses.
        const bool expected_to_fail =
            it != expected_statuses.end() && (it->second == "parse_error" || it->second == "unregistered_type");

        maui::controls::content_page page;
        // Hold the result: it OWNS the created object graph (PROFILE §8 non-owning tree wiring), so
        // discarding it would free the content tree before we inspect it.
        std::string error;
        try
        {
            const xaml_load_result result = xaml_loader::load_into(page, xaml);
            EXPECT_NE(page.content(), nullptr) << "twin hydrated but produced no content tree";
        }
        catch (const maui::xaml::xaml_parse_exception& exception)
        {
            error = exception.unformatted_message();
        }

        if (expected_to_fail)
        {
            EXPECT_FALSE(error.empty()) << "gap page " << GetParam().filename()
                                        << " loaded cleanly — gap closed, update pages/manifest.json's "
                                           "expected_port_status (currently "
                                        << it->second << ")";
        }
        else
        {
            EXPECT_TRUE(error.empty()) << "twin " << GetParam().filename() << " failed to load: " << error;
        }
    }

    // The host-side contract for UNSUPPORTED markup: with a hydration exception_handler installed —
    // C#'s doNotThrow knob (HydrationContext.ExceptionHandler, wired from ResourceLoader.ExceptionHandler2
    // in XamlLoader.cs:101-107/121-147) — a page that names a property/type the loader cannot assign must
    // DEGRADE, not abort: nothing escapes the load, and the surviving tree is still mounted. This is what
    // the gallery hosts install (examples/gallery_xaml/main.cpp + apphost/app_host.cpp); without it the
    // first gap_* page SIGABRTs the app (uncaught xaml_parse_exception -> std::terminate) and a capture
    // lane photographs the launcher.
    //
    // Both halves matter. "Did not throw" alone would pass on a BLANK page, which is the invisible-wrong-
    // capture failure mode this test exists to catch — so the non-null content assertion is the real gate.
    TEST_P(gallery_twin, degrades_instead_of_aborting_when_an_exception_handler_is_installed)
    {
        maui::xaml::register_runtime_bindings();

        const std::string xaml = read_file(GetParam());
        ASSERT_FALSE(xaml.empty()) << "empty twin file: " << GetParam();

        std::vector<std::string> collected;
        maui::xaml::xaml_load_options options;
        options.exception_handler = [&collected](const maui::xaml::xaml_parse_exception& error) {
            collected.push_back(error.what());
        };

        maui::controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, xaml, options);
        EXPECT_NE(page.content(), nullptr)
            << "collected " << collected.size() << " loader error(s) and produced a BLANK page — a host would "
            << "render (and a capture lane would bank) an empty screen"
            << (collected.empty() ? "" : ("; first: " + collected.front()));
    }

    INSTANTIATE_TEST_SUITE_P(gallery_twins, gallery_twin, testing::ValuesIn(twin_files()),
                             [](const testing::TestParamInfo<std::filesystem::path>& info) {
                                 return info.param.stem().string(); // the case name = the file stem
                             });

    // ---- the COMPILE-TIME half of the gap: MAUI_XAML_REJECT_EVENT_ATTRIBUTES -----------------------
    //
    // e2e.py gen bakes that macro into every generated page TU, so an inline event attribute fails the
    // BUILD — mirroring MAUI's own XamlC, which rejects an unbindable event at build time
    // (Build.Tasks/SetPropertiesVisitor.ConnectEvent -> BuildException(MissingEventHandler), :1319).
    //
    // The macro CANNOT be exercised by the shipped TUs: the only page that carries an event attribute is
    // gap_event_attribute, and it is deliberately carved out (emitting the assert there would make the
    // page un-compilable and take the gallery_xaml target, and the Android XAML capture column, with it).
    // So the positive control lives HERE, or the scanner would ship having never once been proven to fire.
    TEST(xaml_static_check, rejects_an_inline_event_attribute_as_a_constant_expression)
    {
        using maui::xaml::find_inline_event_attribute;

        // Constant-expression positive control: this is the shape the generated static_assert evaluates.
        static_assert(find_inline_event_attribute(R"(<Button Clicked="OnClicked" />)").line == 1);
        static_assert(find_inline_event_attribute(R"(<Button Clicked="OnClicked" />)").column == 9);

        // The three shapes a plain substring search gets WRONG — all three occur in the real corpus file
        // (a Clicked=".." inside the comment on line 4, and Clicked=&quot; inside a Label Text on line 15),
        // which is why this is a tokenizer and not a regex. e2e.py's EVENT_RE still has this bug; it is
        // only unexposed because the one page that would trip it is carved out by name.
        static_assert(find_inline_event_attribute(R"(<!-- Clicked="x" --><Label />)").line == 0);
        static_assert(find_inline_event_attribute(R"(<Label Text="Clicked=&quot;x&quot;" />)").line == 0);
        static_assert(find_inline_event_attribute(R"(<Label Text='Clicked="x"' />)").line == 0);
        // A longer name that merely STARTS with an event name is not an event attribute.
        static_assert(find_inline_event_attribute(R"(<Button ClickedTwice="x" />)").line == 0);
        // Whitespace is legal around '=', and the position is the ATTRIBUTE's (see the header note on why
        // this deliberately differs from the loader's runtime column).
        static_assert(find_inline_event_attribute("<Button\n  Tapped = \"h\" />").line == 2);
        static_assert(find_inline_event_attribute("<Button\n  Tapped = \"h\" />").column == 3);
        SUCCEED();
    }

    // The deployability number: across the WHOLE corpus the check must fire on exactly the one deliberate
    // gap page and nowhere else. A false positive here would fail the build of a page that renders fine
    // today, so this is the gate that keeps the generated static_assert safe to ship.
    TEST(xaml_static_check, fires_on_exactly_the_one_gap_page_across_the_corpus)
    {
        std::vector<std::string> hits;
        for (const std::filesystem::path& file : twin_files())
        {
            const maui::xaml::event_attribute_hit hit = maui::xaml::find_inline_event_attribute(read_file(file));
            if (hit.line != 0)
            {
                hits.push_back(file.stem().string() + " " + std::to_string(hit.line) + ":" +
                               std::to_string(hit.column));
            }
        }
        // gap_event_attribute.xaml line 16 is `<Button Text="Click me" Clicked="OnClicked" />`; column 33
        // is the 'C' of Clicked. NOT line 4 (the comment) and NOT line 15 (the &quot;-escaped Label text).
        EXPECT_EQ(hits, (std::vector<std::string>{"gap_event_attribute 16:33"}));
    }
} // namespace
