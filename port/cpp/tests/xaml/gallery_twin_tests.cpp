// Phase-4 corpus gate: every gallery-page XAML twin under examples/gallery_xaml/Views/ must hydrate
// through the runtime loader as a PLAIN ContentPage on the supported surface. This is the achievable
// path to gallery parity — the raw src/Controls/samples pages are views:BasePage-rooted and cannot load
// as-is, so the parity twins re-author each page body under a <ContentPage> root using only registered
// markup. Each .xaml file becomes one parametrized case: it must load with no thrown
// xaml_parse_exception and produce a non-null content tree. Dropping a new twin into the directory adds
// a case automatically (the corpus that the gallery_xaml example #embeds and the iOS comparison shoots).
//
// GALLERY_TWINS_DIR is the absolute source-tree path, injected by CMake (target_compile_definitions).

#include "maui/xaml/xaml_loader.hpp"
#include "maui/xaml/xaml_runtime_bindings.hpp" // register_runtime_bindings (twins with {Binding})

#include "maui/controls/content_page.hpp"
#include "maui/core/i_view.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using maui::xaml::xaml_load_result;
    using maui::xaml::xaml_loader;

    // The twin files, discovered once at suite-instantiation time (std::filesystem glob of the corpus dir).
    [[nodiscard]] std::vector<std::filesystem::path> twin_files()
    {
        std::vector<std::filesystem::path> files;
        const std::filesystem::path dir{GALLERY_TWINS_DIR};
        if (std::filesystem::is_directory(dir))
        {
            for (const auto& entry : std::filesystem::directory_iterator(dir))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".xaml")
                {
                    files.push_back(entry.path());
                }
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    }

    [[nodiscard]] std::string read_file(const std::filesystem::path& path)
    {
        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        return buffer.str();
    }

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
        EXPECT_TRUE(error.empty()) << "twin " << GetParam().filename() << " failed to load: " << error;
    }

    INSTANTIATE_TEST_SUITE_P(gallery_twins, gallery_twin, testing::ValuesIn(twin_files()),
                             [](const testing::TestParamInfo<std::filesystem::path>& info) {
                                 return info.param.stem().string(); // the case name = the file stem
                             });
} // namespace
