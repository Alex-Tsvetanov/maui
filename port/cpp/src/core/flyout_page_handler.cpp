// flyout_page_handler — cross-platform part: the shared mapper tables + ctor + the map functions
// (FlyoutViewHandler.cs's key set). The platform recipe (the native two-pane host, set_panes /
// update_presentation) lives in the per-backend partial; is_gesture_enabled is a mirror on every
// backend (no native swipe-gesture host yet — documented).

#include "maui/core/flyout_page_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_flyout_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // The shared view_mapper chains first, then the FlyoutViewHandler keys. The pane keys come before
    // the presentation keys so a freshly-connected handler hosts the panes before realizing IsPresented
    // (C# puts Flyout/Detail in a priority mapper for the same reason).
    property_mapper<i_view, flyout_page_handler>& flyout_page_handler::mapper()
    {
        static property_mapper<i_view, flyout_page_handler> table{
            view_mapper(),
            {
                {"flyout", &flyout_page_handler::map_panes},
                {"detail", &flyout_page_handler::map_panes},
                {"is_presented", &flyout_page_handler::map_presentation},
                {"flyout_behavior", &flyout_page_handler::map_presentation},
                {"is_gesture_enabled", &flyout_page_handler::map_gesture_enabled},
            },
        };
        return table;
    }

    // No flyout commands (C#'s FlyoutViewHandler.CommandMapper carries only the inherited view
    // commands). The type must be qualified inside the body: the method name shadows the template.
    maui::core::command_mapper<i_view, flyout_page_handler>& flyout_page_handler::command_mapper()
    {
        static maui::core::command_mapper<i_view, flyout_page_handler> table{};
        return table;
    }

    flyout_page_handler::flyout_page_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    // The native split-view controller this handler owns (IPlatformViewHandler.ViewController) — the
    // `controller` slot exists only on the real-native builds (the platform struct's
    // MAUI_PLATFORM_APPLE/IOS guard); headless has no controller, so it returns null and the window host
    // falls back to the plain-view graft. The iOS window host reads this to set the UIWindow's
    // rootViewController to the UISplitViewController (activating its child-VC lifecycle).
    void* flyout_page_handler::root_view_controller() const
    {
#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        auto* platform = typed_platform_view();
        return platform != nullptr ? platform->controller : nullptr;
#else
        return nullptr;
#endif
    }

    // C# MapFlyout / MapDetail: re-host the two panes, then re-realize the presentation (a re-hosted
    // pane set resets the native split state).
    void flyout_page_handler::map_panes(flyout_page_handler& handler, i_view& view)
    {
        handler.set_panes(view);
        handler.update_presentation(view);
    }

    // C# MapIsPresented / MapFlyoutBehavior: realize the presented state + the computed behavior.
    void flyout_page_handler::map_presentation(flyout_page_handler& handler, i_view& view)
    {
        handler.update_presentation(view);
    }

    // C# MapIsGestureEnabled — set the cross-platform mirror, then re-realize the presentation (the
    // ios twin pushes the flag onto presentsWithGesture there; headless/apple keep the mirror).
    void flyout_page_handler::map_gesture_enabled(flyout_page_handler& handler, i_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* flyout = dynamic_cast<i_flyout_view*>(&view))
        {
            platform->gesture_enabled = flyout->flyout_is_gesture_enabled();
        }
        handler.update_presentation(view);
    }
} // namespace maui::core
