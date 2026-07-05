// Phase-4 RENDER gate: every gallery-page XAML twin under examples/gallery_xaml/Views/ must not only LOAD
// (the maui_xaml corpus gate) but MOUNT + LAY OUT through the hosting driver — i.e. it is actually
// renderable, not merely parseable. This is the headless de-risk for the phase-5 iOS visual capture: a
// twin that mounts every handler and arranges to a non-degenerate frame here will render on a device.
//
// Pattern mirrors app_host_tests.cpp exactly: build the app through the builder (which seeds the headless
// controls-handler table in its ctor), mount_window (attach a handler to every element depth-first), then
// drive_layout (measure + arrange). The only twist: the page's content tree is hydrated from the twin's
// markup via xaml_loader::load_into before the mount, so the generic driver mounts the loaded tree.
//
// GALLERY_TWINS_DIR is the absolute source path, injected by CMake (target_compile_definitions).

#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_window.hpp"
#include "maui/xaml/xaml_loader.hpp"
#include "maui/xaml/xaml_runtime_bindings.hpp" // register_runtime_bindings (twins with {Binding})

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    // The markup the next-built render_probe_app will hydrate into its page. gtest runs cases serially, so a
    // single shared slot is safe; the app ctor reads it during build() (before any handler exists — the
    // construction-order case the mount driver's re-host handles).
    std::string g_twin_markup; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    // A minimal hostable app whose content_page is hydrated from g_twin_markup. result_ is declared first so
    // it is destroyed LAST — it OWNS the loaded object graph (PROFILE §8 non-owning tree wiring), which must
    // outlive the page/window that reference it.
    class render_probe_app final : public maui::controls::application
    {
    public:
        render_probe_app()
        {
            result_ = maui::xaml::xaml_loader::load_into(page_, g_twin_markup);
            window_.set_content(page_);
        }

        maui::core::i_window* create_window() override
        {
            return &window_;
        }

        maui::controls::window& win()
        {
            return window_;
        }
        maui::controls::content_page& page()
        {
            return page_;
        }

    private:
        maui::xaml::xaml_load_result result_;
        maui::controls::window window_;
        maui::controls::content_page page_;
    };

    // The page files: the shared-pages dir (port/maui-reference/pages, SHARED_PAGES_DIR) UNION the
    // legacy twins dir, a shared page superseding a same-stem legacy twin (e2e.py gen's precedence).
    [[nodiscard]] std::vector<std::filesystem::path> twin_files()
    {
        std::vector<std::filesystem::path> files;
        std::vector<std::string> seen;
        for (const char* dir_name : {SHARED_PAGES_DIR, GALLERY_TWINS_DIR})
        {
            const std::filesystem::path dir{dir_name};
            if (!std::filesystem::is_directory(dir))
            {
                continue;
            }
            for (const auto& entry : std::filesystem::directory_iterator(dir))
            {
                if (!entry.is_regular_file() || entry.path().extension() != ".xaml")
                {
                    continue;
                }
                const std::string stem = entry.path().stem().string();
                if (std::find(seen.begin(), seen.end(), stem) != seen.end())
                {
                    continue; // shared page already claimed this key
                }
                seen.push_back(stem);
                files.push_back(entry.path());
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

    class gallery_twin_render : public testing::TestWithParam<std::filesystem::path>
    {
    };

    TEST_P(gallery_twin_render, mounts_and_lays_out)
    {
        // Twins may use {Binding} (an ItemTemplate's {Binding .} cell, items/collectionview); install the
        // real runtime-binding applier so they load + render instead of hitting the rejecting default.
        maui::xaml::register_runtime_bindings();

        g_twin_markup = read_file(GetParam());
        ASSERT_FALSE(g_twin_markup.empty()) << "empty twin file: " << GetParam();

        auto app = maui::hosting::maui_app::create_builder().use_maui_app<render_probe_app>().build();
        auto* probe = app->application_as<render_probe_app>().get();
        ASSERT_NE(probe, nullptr);
        ASSERT_NE(probe->page().content(), nullptr) << "twin produced no content tree";

        // Attach a handler to every element in the loaded tree, then measure + arrange at a phone size.
        maui::hosting::mount_window(*app, probe->win());
        const maui::graphics::size arranged = maui::hosting::drive_layout(probe->win(), 402.0, 874.0);

        // Renderable = the mount + arrange produced a non-degenerate layout for the page.
        EXPECT_GT(arranged.width, 0.0) << "twin " << GetParam().filename() << " arranged to zero width";
        EXPECT_GT(arranged.height, 0.0) << "twin " << GetParam().filename() << " arranged to zero height";
        EXPECT_NE(probe->page().handler(), nullptr) << "page handler did not attach";
        EXPECT_GT(probe->page().frame().width, 0.0) << "page frame has zero width";
        EXPECT_GT(probe->page().frame().height, 0.0) << "page frame has zero height";
    }

    INSTANTIATE_TEST_SUITE_P(gallery_twins, gallery_twin_render, testing::ValuesIn(twin_files()),
                             [](const testing::TestParamInfo<std::filesystem::path>& info) {
                                 return info.param.stem().string();
                             });
} // namespace
