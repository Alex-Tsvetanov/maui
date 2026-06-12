#pragma once
// maui::platform::android — the process-wide android.content.Context the Android handlers create
// their widgets from. Internal seam infrastructure for the Android backend, NOT a ported MAUI type
// (the C# counterpart is the IMauiContext.Context every ViewHandler reaches through its
// MauiContext — the port's handler seam has no per-handler context plumbing yet, so the backend
// exposes ONE process-wide Context the way jni_env.hpp exposes the one JavaVM).
//
// Ownership doctrine (PROFILE §8) applied: the slot is a NON-owning borrow of a global reference
// the registrar owns for the process lifetime (the widget test host pins the bootstrap's themed
// Context and deliberately never releases it — see testhost/test_host.cpp; a future real app host
// registers its Application context the same way). Register once, before any handler attaches.

#include <jni.h>

#include <atomic>

namespace maui::platform::android
{
    namespace detail
    {
        inline std::atomic<jobject>& app_context_slot() noexcept
        {
            static std::atomic<jobject> slot{nullptr};
            return slot;
        }
    } // namespace detail

    // Register the process Context (a global reference the caller keeps alive for the process
    // lifetime). Call once, before any android handler creates a platform view.
    inline void set_app_context(jobject context) noexcept
    {
        detail::app_context_slot().store(context, std::memory_order_release);
    }

    // The registered Context, or nullptr when no host registered one (then the android handler
    // partials degrade to their headless-mirror behavior — see button_handler.cpp).
    [[nodiscard]] inline jobject app_context() noexcept
    {
        return detail::app_context_slot().load(std::memory_order_acquire);
    }
} // namespace maui::platform::android
