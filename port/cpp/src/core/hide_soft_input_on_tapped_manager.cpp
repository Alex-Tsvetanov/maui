// hide_soft_input_on_tapped_manager — cross-platform tracking (the ANDROID||IOS-shared
// HideSoftInputOnTappedChangedManager.Platform.cs: UpdatePage / UpdateFocusForView / FeatureEnabled /
// SetupHideSoftInputOnTapped() / DisconnectFromPlatform()). The ONE backend-specific step — wiring the
// native resign-first-responder tap gesture (SetupHideSoftInputOnTapped(UIView) / the .iOS.cs twin) — is
// setup_native(), defined per backend (src/platform/ios/*.mm does the UIKit work; headless + apple return
// an empty token, so this tracking runs but produces no observable native effect off-device). This split
// mirrors gesture_platform_manager (shared core + a single backend hook), so every member field is used
// in this shared TU and every method is a genuine instance method on all backends.

#include "maui/platform/ios/hide_soft_input_on_tapped_manager.hpp"

#include <algorithm>
#include <ranges>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_view.hpp"

namespace maui::platform::ios
{
    hide_soft_input_on_tapped_manager::~hide_soft_input_on_tapped_manager()
    {
        disconnect_from_platform();
    }

    bool hide_soft_input_on_tapped_manager::feature_enabled() const
    {
        // C# FeatureEnabled: any tracked page with the flag set that has navigated to (has_appeared).
        return std::ranges::any_of(content_pages_, [](const tracked_page& tracked) {
            return tracked.page != nullptr && tracked.page->hide_soft_input_on_tapped() && tracked.page->has_appeared();
        });
    }

    void hide_soft_input_on_tapped_manager::update_page(maui::controls::content_page& page)
    {
        // C# UpdatePage: track the page (once) + hook its disappearing (NavigatedFrom) when the flag is set
        // AND it has navigated to; otherwise drop it. Re-run the focus setup either way.
        const bool already_tracked =
            std::ranges::any_of(content_pages_, [&page](const tracked_page& tracked) { return tracked.page == &page; });

        if (page.hide_soft_input_on_tapped() && page.has_appeared())
        {
            if (!already_tracked)
            {
                tracked_page tracked;
                tracked.page = &page;
                tracked.disappearing_token =
                    maui::core::connect_scoped(page.disappearing, [this, &page] { remove_page(page); });
                content_pages_.push_back(std::move(tracked));
                setup_hide_soft_input_on_tapped();
            }
        }
        else
        {
            remove_page(page);
        }
    }

    void hide_soft_input_on_tapped_manager::remove_page(maui::controls::content_page& page)
    {
        // C# RemovePage / OnPageNavigatedFrom: drop the page (its disappearing hook tears down with the
        // erased tracked_page's scoped_connection) and re-run the focus setup.
        const auto removed = std::ranges::remove_if(
            content_pages_, [&page](const tracked_page& tracked) { return tracked.page == &page; });
        content_pages_.erase(removed.begin(), removed.end());
        setup_hide_soft_input_on_tapped();
    }

    void hide_soft_input_on_tapped_manager::update_focus_for_view(maui::core::i_view& view)
    {
        // C# UpdateFocusForView: re-point / clear the tracked focused view based on its focus state.
        if (view.is_focused())
        {
            disconnect_from_platform();
            focused_view_ = &view;
        }
        else if (&view == focused_view_)
        {
            disconnect_from_platform();
            focused_view_ = nullptr;
        }

        if (!feature_enabled())
        {
            disconnect_from_platform();
            return;
        }

        if (!view.is_focused())
        {
            return;
        }

        disconnect_from_platform();

        // The native gesture wiring (C# SetupHideSoftInputOnTapped(platformView)); the handler-not-attached
        // / no-window guards live inside setup_native (which returns an empty token there, and on every
        // off-device backend).
        watching_for_taps_ = setup_native(view);
    }

    void hide_soft_input_on_tapped_manager::setup_hide_soft_input_on_tapped()
    {
        // C# SetupHideSoftInputOnTapped(): re-evaluate the wiring for the currently-tracked focused view.
        if (focused_view_ != nullptr)
        {
            update_focus_for_view(*focused_view_);
        }
    }

    void hide_soft_input_on_tapped_manager::disconnect_from_platform() noexcept
    {
        // C# DisconnectFromPlatform(): dispose + clear the watching-for-taps cleanup. Move the closure out
        // first (so a re-entrant call sees it cleared), then run it under a swallowing guard: the backend
        // cleanup (gesture-recognizer teardown via ObjC) never throws, but std::function::operator() is
        // nominally throwing, and this must stay noexcept because the destructor calls it.
        const auto cleanup = std::exchange(watching_for_taps_, nullptr);
        if (cleanup)
        {
            try
            {
                cleanup();
            }
            // A teardown must never propagate out of the destructor; there is nothing to recover.
            // NOLINTNEXTLINE(bugprone-empty-catch) -- intentional swallow (cf. src/graphics/path_builder.cpp)
            catch (...)
            {
            }
        }
    }
} // namespace maui::platform::ios
