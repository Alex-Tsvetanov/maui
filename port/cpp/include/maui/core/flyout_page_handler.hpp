#pragma once
// maui::core::flyout_page_handler  <=  Microsoft.Maui.Handlers.FlyoutViewHandler
//
// The handler behind a flyout_page: it owns a native two-pane host and keeps the flyout pane, the
// detail pane, and the presented state in sync with the virtual view (reached through the
// maui::core::i_flyout_view seam). The C# cross-platform FlyoutViewHandler maps Flyout / Detail /
// IsPresented / FlyoutBehavior / IsGestureEnabled (the Android/Windows mapper set — the iOS chrome
// lives in the compat PhoneFlyoutPageRenderer); the port keeps those KEYS as ordinary mapper entries on
// every backend.
//
// Per-backend platform recipe (the partial split):
//   headless — mirrors only (hosted_flyout / hosted_detail / presented / behavior / gesture_enabled).
//   ios      — a real UISplitViewController (the C# task's controller-hierarchy assertion target): the
//              flyout and detail panes are wrapped in child UIViewControllers (primary/secondary),
//              IsPresented drives preferredDisplayMode (oneBesideSecondary vs secondaryOnly), Locked
//              pins the flyout beside the detail. The native→virtual presented sync seam is
//              i_flyout_view::set_flyout_is_presented (the displayModeButton/show-hide callbacks need a
//              live UIWindow, which the bundle-less test process cannot host — documented deviation;
//              the seam itself is exercised directly).
//   apple    — a real NSSplitViewController: a sidebar NSSplitViewItem hosts the flyout pane and a
//              content item the detail; IsPresented drives the sidebar item's `collapsed`. One-way
//              virtual→native (a user drag-collapse is not observed back — documented deviation).

#include <memory>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/flyout_behavior.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto it
    // (headless keeps the base mirrors; the real backends override update_* on the split host's view).
    struct flyout_page_platform : view_platform_base
    {
        flyout_page_platform() = default;
        ~flyout_page_platform() override; // backend-defined: releases the retained native views
        flyout_page_platform(const flyout_page_platform&) = delete;
        flyout_page_platform(flyout_page_platform&&) = delete;
        flyout_page_platform& operator=(const flyout_page_platform&) = delete;
        flyout_page_platform& operator=(flyout_page_platform&&) = delete;

        void* native = nullptr; // the split host's root view (the controller's .view on real backends)

        // The seam mirrors (headless-observable; the real backends ALSO drive the controller):
        i_view* hosted_flyout = nullptr; // the flyout pane's page
        i_view* hosted_detail = nullptr; // the detail pane's page
        bool presented = false;          // IsPresented as realized natively
        flyout_behavior behavior = flyout_behavior::flyout;
        bool gesture_enabled = true;

#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        // The retained native slots shared by the two real-native twins.
        void* controller = nullptr;  // NSSplitViewController / UISplitViewController (retained)
        void* flyout_host = nullptr; // the flyout pane's wrapper view controller (retained)
        void* detail_host = nullptr; // the detail pane's wrapper view controller (retained)
#endif

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the split controller's NSView (defined in
        // src/platform/apple/flyout_page_handler.mm). is_enabled keeps the base mirror (plain NSView).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the generic IView properties to the split controller's UIView (defined in
        // src/platform/ios/flyout_page_handler.mm). is_enabled keeps the base mirror (plain UIView).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class flyout_page_handler : public view_handler<flyout_page_handler, i_view, flyout_page_platform>
    {
    public:
        flyout_page_handler();

        static property_mapper<i_view, flyout_page_handler>& mapper();
        static command_mapper<i_view, flyout_page_handler>& command_mapper();

        static std::unique_ptr<flyout_page_platform> create_platform_view();

        // The native UISplitViewController this handler owns (the platform's `controller` slot), so the
        // window host can set the UIWindow's rootViewController to it — activating the split's child-VC
        // lifecycle (without which the panes never lay out / render). Mirrors C#'s
        // IPlatformViewHandler.ViewController. iOS-only (the `controller` slot exists on the real-native
        // builds); returns null on headless. Defined per backend.
        [[nodiscard]] void* root_view_controller() const override;

        // The flyout page computes its size from its panes, not the handler (the container convention).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Re-host the two panes from the view's current Flyout/Detail (defined per backend).
        void set_panes(i_view& view);
        // Realize IsPresented + the computed FlyoutBehavior natively (defined per backend).
        void update_presentation(i_view& view);

        // "flyout" / "detail" — C# MapFlyout / MapDetail (both re-host the pane set).
        static void map_panes(flyout_page_handler& handler, i_view& view);
        // "is_presented" / "flyout_behavior" — C# MapIsPresented / MapFlyoutBehavior.
        static void map_presentation(flyout_page_handler& handler, i_view& view);
        // "is_gesture_enabled" — C# MapIsGestureEnabled (mirror; no native gesture host yet).
        static void map_gesture_enabled(flyout_page_handler& handler, i_view& view);
    };
} // namespace maui::core
