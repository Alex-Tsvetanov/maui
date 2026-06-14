#pragma once
// maui::core::i_ios_page_specifics  <=  Microsoft.Maui.Platform.IiOSPageSpecifics
//
// The Core-layer face of the iOSSpecific Page knobs (W2-24): the window's root UIViewController asks
// the hosted page for its status-bar / home-indicator preferences through this contract instead of
// knowing the Controls-layer attached store (C# PageViewController reads it off CurrentView the same
// way). Implemented by controls::content_page over the platform-spec store. The two mode getters are
// raw ints exactly like C# (the switch in the consumer maps 1/2 → hidden/shown and 0/1 → fade/slide);
// the enum types live in the Controls layer (iOSSpecific) and Core must not depend on them.
//
// The matching attached-store keys (kept in sync with
// include/maui/controls/platform_configuration/ios_specific/page.hpp — C#'s cross-assembly nameof
// analog) are the literals "ios.Page.PrefersStatusBarHidden" / "ios.Page.PrefersHomeIndicatorAutoHidden"
// the content-page handler maps (content_page_handler.cpp).

namespace maui::core
{
    class i_ios_page_specifics
    {
    public:
        virtual ~i_ios_page_specifics() = default;

        // C# IiOSPageSpecifics.IsHomeIndicatorAutoHidden.
        [[nodiscard]] virtual bool is_home_indicator_auto_hidden() const = 0;

        // C# IiOSPageSpecifics.PrefersStatusBarHiddenMode (StatusBarHiddenMode as int: 0 Default,
        // 1 True → hidden, 2 False → shown).
        [[nodiscard]] virtual int prefers_status_bar_hidden_mode() const = 0;

        // C# IiOSPageSpecifics.PreferredStatusBarUpdateAnimationMode (UIStatusBarAnimation as int:
        // 0 None, 1 Slide, 2 Fade — note the consumer's C# switch maps 0 → Fade, 1 → Slide, else None,
        // an oracle quirk ported verbatim in the window root controller).
        [[nodiscard]] virtual int preferred_status_bar_update_animation_mode() const = 0;

    protected:
        i_ios_page_specifics() = default;
        i_ios_page_specifics(const i_ios_page_specifics&) = default;
        i_ios_page_specifics(i_ios_page_specifics&&) = default;
        i_ios_page_specifics& operator=(const i_ios_page_specifics&) = default;
        i_ios_page_specifics& operator=(i_ios_page_specifics&&) = default;
    };
} // namespace maui::core
