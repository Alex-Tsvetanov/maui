// Diagnostic for the border_playground/border_stroke maccatalyst parity investigation (2026-08-24).
// The maccatalyst board shows "Border Content" (Label) + its Picker vanishing entirely from the
// render (both cpp and xaml columns), with everything below shifted up by their combined height. This
// test establishes the CONTROL and RULES OUT the shared core measure/arrange pipeline
// (vertical_stack_layout_manager.cpp) as the cause: on both `headless` (pure core) and `apple` (a REAL
// native NSPopUpButton's own fittingSize), the two children measure and arrange to nonzero, correctly
// stacked heights. So the defect is NOT a measure bug here.
//
// NO COMPANION TEST EXISTS FOR THE REMAINING HYPOTHESIS. The leading candidate is a post-arrange
// native scroll-position shift in the `#if TARGET_OS_MACCATALYST` settle block of
// scroll_view_handler::platform_arrange (src/platform/ios/scroll_view_handler.mm, ~554-620, added by
// dd4ecf1ad8) parking the ScrollView at a nonzero contentOffset.y for this page's content height. That
// is UNVERIFIED -- mounting this page's real Catalyst window throws
// NSInternalInconsistencyException ("NSApplication has not been created yet") outside a launched .app
// bundle, and there is no `maccatalyst` ctest preset to exercise it headlessly. Confirming it needs a
// live MAUI_GEOMETRY_DUMP=1 measurement on the Catalyst VM (see dd4ecf1ad8's own methodology), not a
// unit test. Treat it as a LEAD, not a finding, until someone takes that measurement.
#include "maui/hosting/app_host.hpp"

#include <memory>

#include <gtest/gtest.h>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_container.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_window.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

#include "pages/border_playground_page.hpp"

namespace
{
    class border_playground_app final : public maui::controls::application
    {
    public:
        border_playground_app()
        {
            window_.set_content(page_.page());
        }

        maui::core::i_window* create_window() override
        {
            return &window_;
        }

        maui::samples::border_playground_page page_;
        maui::controls::window window_;
    };

    // The maccatalyst gallery window is captured at 1024x800 (see docs/comparison/captures/maccatalyst).
    TEST(border_playground_layout, controls_stack_first_two_children_are_nonzero_height)
    {
        auto app = maui::hosting::maui_app::create_builder().use_maui_app<border_playground_app>().build();
        auto* probe = app->application_as<border_playground_app>().get();
        ASSERT_NE(probe, nullptr);

        maui::hosting::mount_window(*app, probe->window_);
        maui::hosting::drive_layout(probe->window_, 1024.0, 800.0);

        // grid_.at(0) = border_view_, grid_.at(1) = scroller_.
        auto& grid = probe->page_.grid();
        ASSERT_EQ(grid.count(), 2) << "border_playground's Grid should have exactly 2 children";
        auto* scroller = dynamic_cast<maui::core::i_content_view*>(&grid.at(1));
        ASSERT_NE(scroller, nullptr) << "grid child 1 is not a content view (expected the ScrollView)";
        auto* controls = dynamic_cast<maui::core::i_container*>(scroller->content());
        ASSERT_NE(controls, nullptr) << "ScrollView content is not a container (expected the VerticalStackLayout)";

        ASSERT_GT(controls->count(), 1) << "controls stack has fewer than 2 children";
        const maui::graphics::rect caption_frame = controls->at(0).frame();
        const maui::graphics::rect picker_frame = controls->at(1).frame();

        // Print every frame for visibility when this fails / for manual inspection.
        for (int n = 0; n < controls->count() && n < 4; ++n)
        {
            const maui::graphics::rect f = controls->at(n).frame();
            std::fprintf(stderr, "controls[%d] frame = {x=%.2f y=%.2f w=%.2f h=%.2f}\n", n, f.x, f.y, f.width,
                         f.height);
        }

        // If this measure/arrange pipeline produced zero height here (on headless -- shared core -- or
        // on `apple`, a REAL native NSPopUpButton), the maccatalyst board defect would be a measure bug.
        // It does not (see the file header): both children measure and arrange to nonzero height, so the
        // real cause is a post-arrange mechanism this test cannot see -- NOT confirmed by anything else.
        EXPECT_GT(caption_frame.height, 0.0) << "content_caption_ (\"Border Content\" Label) measured to "
                                                "zero height in core layout";
        EXPECT_GT(picker_frame.height, 0.0) << "content_picker_ (\"Border Content\" Picker) measured to "
                                               "zero height in core layout";
    }

    // maccatalyst dark-theme regression (2026-08-24): BorderLineJoinPicker/BorderLineCapPicker's Title
    // (port/maui-reference/pages/border_playground.xaml:73,82) was never set by build_border_shape_row,
    // so the platform placeholder text (view.title()) was empty -- rendering as a blank box rather than
    // "black text on black background". Not a colour bug: PickerExtensions.cs's UpdatePickerTitle (C#
    // oracle) passes a nil foreground when TitleColor is unset, which correctly defers to the system's
    // theme-adaptive placeholder color; that logic was already correct on both backends.
    TEST(border_playground_layout, line_join_and_line_cap_pickers_carry_their_title)
    {
        maui::samples::border_playground_page page;
        EXPECT_EQ(page.line_join_picker().title(), "Border LineJoin");
        EXPECT_EQ(page.line_cap_picker().title(), "Border LineCap");
    }
} // namespace
