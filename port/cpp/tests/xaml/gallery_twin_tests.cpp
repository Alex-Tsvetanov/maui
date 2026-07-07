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

    INSTANTIATE_TEST_SUITE_P(gallery_twins, gallery_twin, testing::ValuesIn(twin_files()),
                             [](const testing::TestParamInfo<std::filesystem::path>& info) {
                                 return info.param.stem().string(); // the case name = the file stem
                             });
} // namespace
