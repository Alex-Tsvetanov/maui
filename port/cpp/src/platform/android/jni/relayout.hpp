#pragma once
// maui::platform::android — the process-wide "re-run the page layout pass" hook. Internal seam
// infrastructure for the Android backend, NOT a ported MAUI type; the sibling of app_context.hpp's
// one-Context slot and installed the same way (once, by the host).
//
// WHY IT EXISTS: MAUI's Android views re-measure themselves. LayoutViewGroup.OnMeasure calls
// CrossPlatformMeasure on EVERY system traversal (src/Core/src/Platform/Android/LayoutViewGroup.cs),
// so when Glide finishes a remote download and ImageView.SetImageDrawable calls requestLayout(), the
// traversal that follows re-measures the cross-platform tree and the freshly-sized image pushes its
// siblings down. The port's MauiLayout.onMeasure deliberately does NOT measure children (the panel's
// size is owned by its handler's platform_arrange), and MauiLayout.onLayout only ARRANGES — using each
// child's CACHED desired_size. So a result that lands AFTER the host's one-shot drive_layout has no way
// to grow its slot: the bitmap reaches the ImageView but the ImageView is still framed at the size it
// measured to while empty.
//
// Rather than make every layout traversal re-measure (a change under every one of the port's pages),
// the host registers ONE callback that replays its own mount-time layout pass, and the few places that
// can legitimately change a view's desired size after mount call request_relayout(). Today that is the
// android image handler's async uri decode (image_handler.cpp's apply_loaded_result).
//
// THREADING: UI-thread only. The hook is installed by the host on the UI thread right after its first
// drive_layout, and every caller reaches it from the UI thread too (the uri fetch marshals its
// completion through Handler(Looper.getMainLooper()) before the decode+apply runs). No slot is shared
// across threads, so the plain std::function needs no synchronization.
//
// Unset by default: the widget test host, the cross-platform suite and any VM-less path never install
// one, so request_relayout() is a no-op there — the same degradation shape as an absent app_context().

#include <functional>
#include <utility>

namespace maui::platform::android
{
    namespace detail
    {
        inline std::function<void()>& relayout_slot()
        {
            static std::function<void()> slot;
            return slot;
        }
    } // namespace detail

    // Register the host's layout pass (its drive_layout over the display bounds). Call once, on the UI
    // thread, after the initial mount; the captured state must outlive the process' UI (the android app
    // hosts leak their maui_app deliberately, so their window reference stays valid).
    inline void set_relayout_hook(std::function<void()> hook)
    {
        detail::relayout_slot() = std::move(hook);
    }

    // Re-run the host's layout pass, or nothing when no host registered one.
    inline void request_relayout()
    {
        if (const std::function<void()>& hook = detail::relayout_slot())
        {
            hook();
        }
    }
} // namespace maui::platform::android
