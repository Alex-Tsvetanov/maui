#pragma once
// maui::samples::input_transparent_page — ports InputTransparentPage.xaml
//                                         (Maui.Controls.Sample.Pages.InputTransparentPage).
//
// The C# page demonstrates VisualElement.InputTransparent: a Button with InputTransparent=true is
// invisible to input (a tap passes THROUGH it to whatever is underneath), while InputTransparent=false
// receives the tap. The page stacks pairs of buttons in grids and wires every clickable one to
// ClickSuccess and every "should not be clickable" one to ClickFail, so an alert reports whether input
// routed correctly. It also has a switch panel that toggles InputTransparent (and CascadeInputTransparent)
// on a nested Root/Nested/Button hierarchy live.
//
// This port reproduces the input-routing behavior code-first and surfaces it through a readout (the
// gallery convention, in place of the C# DisplayAlert). Because the headless backend has no native
// hit-tester, the page models the SAME semantics deterministically: simulate_tap() walks a stack of
// overlapping buttons from top to bottom and delivers the tap to the FIRST one whose input_transparent()
// is false — exactly what a real hit-test does with InputTransparent. The readout reports which button
// got it (Success / Failure, mirroring ClickSuccess / ClickFail).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// hosts page() in a window; the headless test tree exercises the same wiring deterministically.
//
// Interactions demonstrated:
//   - a single clickable button (InputTransparent=false) takes the tap -> Success,
//   - a single InputTransparent=true button: a tap on it passes through (no receiver) -> Failure note,
//   - an overlay: a top InputTransparent=true button over a clickable bottom button — the tap routes to
//     the bottom button (Success), proving pass-through,
//   - toggling InputTransparent on the "test button" via a switch flips which layer receives the tap,
//     re-running simulate_tap() to show the live change (the C# switch panel).
//
// note: C# CascadeInputTransparent (a layout propagating InputTransparent to its descendants) is NOT in
//       the headless port's view surface (only the per-view InputTransparent bindable exists). So the
//       cascade-specific grids are best-effort: this port demonstrates the per-view InputTransparent
//       routing (the core property) and toggles it directly on each layer; the cascade variant is left as
//       a // note rather than invented.
// note: the headless readout replaces the C# DisplayAlert (no modal alert seam on the headless backend);
//       the routing decision (which button receives input) is identical.

#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class input_transparent_page
    {
    public:
        input_transparent_page()
        {
            page_.set_title("Input transparent");
            stack_.set_spacing(8);
            readout_.set_text("Ready — tap a layer set below");

            // ---- section 1: a single clickable button (InputTransparent=false) -------------------------
            single_clickable_label_.set_text("Single button, InputTransparent=false: should be clickable.");
            clickable_btn_.set_text("Clickable");
            clickable_btn_.set_input_transparent(false);
            clickable_btn_.clicked.connect([this] { click_success("Clickable"); });
            single_tap_btn_.set_text("Tap single clickable");
            single_tap_btn_.clicked.connect([this] { simulate_tap("single", {&clickable_btn_}); });

            // ---- section 2: a single InputTransparent=true button (tap passes through to nothing) -------
            single_transparent_label_.set_text(
                "Single button, InputTransparent=true: should NOT be clickable (tap passes through).");
            not_clickable_btn_.set_text("Not Clickable");
            not_clickable_btn_.set_input_transparent(true);
            not_clickable_btn_.clicked.connect([this] { click_fail("Not Clickable"); });
            transparent_tap_btn_.set_text("Tap single transparent");
            transparent_tap_btn_.clicked.connect([this] { simulate_tap("transparent", {&not_clickable_btn_}); });

            // ---- section 3: overlay — transparent top over a clickable bottom (pass-through) -----------
            overlay_label_.set_text(
                "Overlay: top InputTransparent=true over a clickable bottom; tap routes to the bottom.");
            overlay_bottom_btn_.set_text("Bottom (clickable)");
            overlay_bottom_btn_.set_input_transparent(false);
            overlay_bottom_btn_.clicked.connect([this] { click_success("Bottom (clickable)"); });
            overlay_top_btn_.set_text("Top (transparent)");
            overlay_top_btn_.set_input_transparent(true);
            overlay_top_btn_.clicked.connect([this] { click_fail("Top (transparent)"); });
            // The grid stacks both in the same cell (top added last == visually on top); the tap walks
            // top->bottom, so [top, bottom] is the hit-test order.
            overlay_grid_.add(overlay_bottom_btn_);
            overlay_grid_.add(overlay_top_btn_);
            overlay_tap_btn_.set_text("Tap overlay");
            overlay_tap_btn_.clicked.connect(
                [this] { simulate_tap("overlay", {&overlay_top_btn_, &overlay_bottom_btn_}); });

            // ---- section 4: the live toggle — a test button over a bottom button -----------------------
            toggle_label_.set_text(
                "Toggle: flip the test button's InputTransparent; the tap re-routes between the two layers.");
            test_bottom_btn_.set_text("Bottom Layer");
            test_bottom_btn_.set_input_transparent(false);
            test_bottom_btn_.clicked.connect([this] { click_success("Bottom Layer"); });
            test_button_.set_text("Test Button");
            test_button_.set_input_transparent(false); // starts opaque -> it receives the tap
            test_button_.clicked.connect([this] { click_success("Test Button"); });
            test_grid_.add(test_bottom_btn_);
            test_grid_.add(test_button_);

            // The switch flips test_button_.InputTransparent (C#'s OneWayToSource Switch -> the property),
            // then re-runs the tap so the readout shows the live re-routing.
            test_transparent_switch_.toggled.connect([this](bool is_on) {
                test_button_.set_input_transparent(is_on);
                simulate_tap("toggle", {&test_button_, &test_bottom_btn_});
            });
            toggle_tap_btn_.set_text("Tap toggle set");
            toggle_tap_btn_.clicked.connect([this] { simulate_tap("toggle", {&test_button_, &test_bottom_btn_}); });

            // ---- assemble ------------------------------------------------------------------------------
            stack_.add(single_clickable_label_);
            stack_.add(clickable_btn_);
            stack_.add(single_tap_btn_);
            stack_.add(single_transparent_label_);
            stack_.add(not_clickable_btn_);
            stack_.add(transparent_tap_btn_);
            stack_.add(overlay_label_);
            stack_.add(overlay_grid_);
            stack_.add(overlay_tap_btn_);
            stack_.add(toggle_label_);
            stack_.add(test_grid_);
            stack_.add(test_transparent_switch_);
            stack_.add(toggle_tap_btn_);
            stack_.add(readout_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED VIEW bottom-up (leaves, then the two grids, then the stack, then
        // the page), then re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, single_clickable_label_, "single_clickable_label_");
            gallery_attach_one(app, clickable_btn_, "clickable_btn_");
            gallery_attach_one(app, single_tap_btn_, "single_tap_btn_");
            gallery_attach_one(app, single_transparent_label_, "single_transparent_label_");
            gallery_attach_one(app, not_clickable_btn_, "not_clickable_btn_");
            gallery_attach_one(app, transparent_tap_btn_, "transparent_tap_btn_");
            gallery_attach_one(app, overlay_label_, "overlay_label_");
            gallery_attach_one(app, overlay_bottom_btn_, "overlay_bottom_btn_");
            gallery_attach_one(app, overlay_top_btn_, "overlay_top_btn_");
            gallery_attach_one(app, overlay_grid_, "overlay_grid_");
            gallery_attach_one(app, overlay_tap_btn_, "overlay_tap_btn_");
            gallery_attach_one(app, toggle_label_, "toggle_label_");
            gallery_attach_one(app, test_bottom_btn_, "test_bottom_btn_");
            gallery_attach_one(app, test_button_, "test_button_");
            gallery_attach_one(app, test_grid_, "test_grid_");
            gallery_attach_one(app, test_transparent_switch_, "test_transparent_switch_");
            gallery_attach_one(app, toggle_tap_btn_, "toggle_tap_btn_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(overlay_grid_);
            gallery_rehost_layout(test_grid_);
            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned views, exposed for the hosting main + headless tests.
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::button& test_button()
        {
            return test_button_;
        }
        [[nodiscard]] maui::controls::button& test_bottom_button()
        {
            return test_bottom_btn_;
        }
        [[nodiscard]] maui::controls::toggle_switch& test_transparent_switch()
        {
            return test_transparent_switch_;
        }

        // The input-routing model itself, exposed so headless tests can assert routing without going
        // through a button: deliver a tap to a top->bottom stack and return the receiver's text (or empty
        // when the tap passes through every layer). Public so it is the testable core of the page.
        [[nodiscard]] static maui::controls::button* hit_test(const std::vector<maui::controls::button*>& top_to_bottom)
        {
            for (maui::controls::button* const candidate : top_to_bottom)
            {
                if (candidate != nullptr && !candidate->input_transparent())
                {
                    return candidate; // first opaque (non-input-transparent) layer wins
                }
            }
            return nullptr; // tap passed through every layer
        }

    private:
        // simulate_tap: route a tap through a top->bottom stack and report on the readout (the C#
        // DisplayAlert stand-in). The receiver's own clicked handler also fires (success/fail), exactly
        // as a real tap would invoke send_clicked() on the hit view.
        void simulate_tap(const std::string& section, const std::vector<maui::controls::button*>& top_to_bottom)
        {
            maui::controls::button* const receiver = hit_test(top_to_bottom);
            if (receiver != nullptr)
            {
                receiver->send_clicked(); // delivers to the hit view (drives its clicked -> success/fail)
            }
            else
            {
                readout_.set_text("[" + section + "] tap passed through all layers (no receiver) — Failure");
            }
        }

        void click_success(const std::string& which)
        {
            readout_.set_text("Success — '" + which + "' received the tap, as it should.");
        }
        void click_fail(const std::string& which)
        {
            readout_.set_text("Failure — '" + which + "' received a tap it should not have.");
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;

        maui::controls::label single_clickable_label_;
        maui::controls::button clickable_btn_;
        maui::controls::button single_tap_btn_;

        maui::controls::label single_transparent_label_;
        maui::controls::button not_clickable_btn_;
        maui::controls::button transparent_tap_btn_;

        maui::controls::label overlay_label_;
        maui::controls::grid overlay_grid_;
        maui::controls::button overlay_bottom_btn_;
        maui::controls::button overlay_top_btn_;
        maui::controls::button overlay_tap_btn_;

        maui::controls::label toggle_label_;
        maui::controls::grid test_grid_;
        maui::controls::button test_bottom_btn_;
        maui::controls::button test_button_;
        maui::controls::toggle_switch test_transparent_switch_;
        maui::controls::button toggle_tap_btn_;
    };
} // namespace maui::samples
