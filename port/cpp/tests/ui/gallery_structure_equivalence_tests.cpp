// Builder-vs-XAML structure equivalence — the first line of defense for the shared-XAML pilot
// (port/maui-reference/pages, manifest rows with builder_twin=true): the code-first builder page
// (examples/gallery/pages/<key>_page.hpp) and the SAME page hydrated from the canonical shared .xaml
// via the runtime loader must normalize to the SAME control tree (tests/support/view_tree_describe.hpp).
//
// One TEST per pilot key; adding a key is one STRUCTURE_EQUIVALENCE_TEST line (plus its page include).
// These tests are EXPECTED to fail while a hand-authored twin genuinely diverges from its builder page —
// each failure's printed tree diff is the work item; do NOT weaken describe() to paper over a real
// structural divergence (only over cosmetic noise, per the header's conservative-props policy).
//
// The builder pages construct plain cross-platform control trees (no handlers, no mounting), so this
// runs headless. SHARED_PAGES_DIR is the absolute source-tree path to the shared pages, injected by
// CMake exactly like tests/xaml/gallery_twin_tests.cpp.

#include "tests/support/view_tree_describe.hpp"

#include "maui/controls/content_page.hpp"
#include "maui/xaml/xaml_loader.hpp"
#include "maui/xaml/xaml_runtime_bindings.hpp" // register_runtime_bindings ({Binding} in item templates)

#include "pages/border_page.hpp"
#include "pages/button_page.hpp"
#include "pages/collectionview_page.hpp"
#include "pages/entry_page.hpp"
#include "pages/gradient_page.hpp"
#include "pages/grid_page.hpp"
#include "pages/image_page.hpp"
#include "pages/label_page.hpp"
#include "pages/scroll_view_page.hpp"
#include "pages/slider_page.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace
{
    using maui::tests::describe;
    using maui::tests::view_node;

    // Hydrate the shared page for `key` through the runtime XAML loader and normalize it. The
    // xaml_load_result OWNS the created object graph (PROFILE §8 non-owning tree wiring), so the tree
    // is described before the result leaves scope.
    [[nodiscard]] view_node describe_shared_xaml(const std::string& key)
    {
        // Idempotent (just sets the global applier); some twins use {Binding} in item templates.
        maui::xaml::register_runtime_bindings();

        const std::filesystem::path path = std::filesystem::path{SHARED_PAGES_DIR} / (key + ".xaml");
        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        const std::string xaml = buffer.str();
        EXPECT_FALSE(xaml.empty()) << "missing or empty shared page: " << path;

        maui::controls::content_page page;
        const maui::xaml::xaml_load_result result = maui::xaml::xaml_loader::load_into(page, xaml);
        return describe(page);
    }

// One structural-equivalence case: builder page `page_type` vs shared `<key>.xaml`. The failure
// message prints both normalized trees (view_node's PrintTo), so the divergence reads as a tree diff.
#define STRUCTURE_EQUIVALENCE_TEST(key, page_type)                                                                     \
    TEST(gallery_structure_equivalence, key)                                                                           \
    {                                                                                                                  \
        maui::samples::page_type builder;                                                                              \
        EXPECT_EQ(describe(builder.page()), describe_shared_xaml(#key));                                               \
    }

    STRUCTURE_EQUIVALENCE_TEST(border, border_page)
    STRUCTURE_EQUIVALENCE_TEST(button, button_page)
    STRUCTURE_EQUIVALENCE_TEST(collectionview, collectionview_page)
    STRUCTURE_EQUIVALENCE_TEST(entry, entry_page)
    STRUCTURE_EQUIVALENCE_TEST(gradient, gradient_page)
    STRUCTURE_EQUIVALENCE_TEST(grid, grid_page)
    STRUCTURE_EQUIVALENCE_TEST(image, image_page)
    STRUCTURE_EQUIVALENCE_TEST(label, label_page)
    STRUCTURE_EQUIVALENCE_TEST(scroll_view, scroll_view_page)
    STRUCTURE_EQUIVALENCE_TEST(slider, slider_page)

#undef STRUCTURE_EQUIVALENCE_TEST
} // namespace
