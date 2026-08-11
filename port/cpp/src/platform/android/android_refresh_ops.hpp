#pragma once
// Shared Android (JNI) pull-to-refresh seam — the C++ half of dev.mauicpp.MauiRefreshBridge.
//
// WHAT THIS IS FOR
// MAUI's Android RefreshView is MauiSwipeRefreshLayout : AndroidX.SwipeRefreshLayout
// (src/Core/src/Platform/Android/MauiSwipeRefreshLayout.cs:17), and the completed pull arrives through
// SwipeRefreshLayout.OnRefreshListener — a Java INTERFACE. JNI cannot implement a Java interface, so the
// listener lives in Java (java/MauiRefreshBridge.java) and calls back here, exactly like the dialog and
// items seams beside this file.
//
// WHY A SEPARATE HEADER FROM android_dialog_ops.hpp / android_items_ops.hpp
// Same reason those two are separate from each other: they share a discipline, not a lifetime. One
// registry per seam means a torn-down picker and a live RefreshView never contend for the same table, and
// each seam's invariants stay readable on their own. This one is the SMALLEST of the three — a single
// callback, fired at most once per gesture — so it carries the four rules and nothing else.
//
// THE FOUR LIFETIME RULES, copied from android_items_ops.hpp / android_dialog_ops.hpp because they were
// learned the hard way there:
//   1. the jlong Java carries is NEVER dereferenced — it is a monotonic id looked up in the registry
//      below, so a stale token resolves to nothing rather than to freed storage or, worse, to a RECYCLED
//      address that happens to hold a different live peer;
//   2. resolving hands back a STRONG shared_ptr held for the whole callback body, so a re-entrant
//      teardown cannot free the trampoline under the running callback;
//   3. the std::function is COPIED out before it is invoked, so a teardown that clears the callbacks
//      mid-call does not destroy the target that is executing;
//   4. the body performs its re-entrant call LAST and touches nothing afterwards.
// Rule 1 is not theoretical here: a SwipeRefreshLayout is retained by the Java view tree and can outlive
// the handler that built it, so a late onRefresh is a NORMAL event, not a pathological one.
//
// VM-LESS: every entry point tolerates a null peer / no JavaVM and quietly does nothing, so the android
// preset's pure-native suite (no VM) runs through this file unchanged.

#include <jni.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"

namespace maui::platform::android
{
    // Monotonic, process-wide, never reused — see rule 1. Its own id space; the dialog and items
    // registries are independent tables with independent counters.
    [[nodiscard]] inline std::uint64_t next_refresh_peer_id()
    {
        static std::atomic<std::uint64_t> next{1}; // 0 stays reserved for "no peer"
        return next.fetch_add(1, std::memory_order_relaxed);
    }

    // The trampoline target: one per connected refresh handler, heap-allocated so a callback can hold it
    // alive independently of the handler.
    struct refresh_trampoline
    {
        // SwipeRefreshLayout.OnRefreshListener.onRefresh — MAUI's OnSwipeRefresh, whose whole body is
        // `VirtualView.IsRefreshing = true` (RefreshViewHandler.Android.cs:24-27). Deliberately still
        // EMPTY when the peer is minted: create_platform_view builds and wires the whole native stack
        // before a handler exists to bind, and on_connect_handler fills this in. An empty callback is a
        // silent no-op, which is the correct behaviour for a view not yet in the tree.
        std::function<void()> on_refresh;

        // Set by clear(): the owning handler has torn down. A callback that has ALREADY re-entered user
        // code reads this — through the strong ref it holds — to decide whether touching the handler
        // again is still legal.
        bool dead = false;

        const std::uint64_t id = next_refresh_peer_id();

        refresh_trampoline() = default;
        refresh_trampoline(const refresh_trampoline&) = delete;
        refresh_trampoline(refresh_trampoline&&) = delete;
        refresh_trampoline& operator=(const refresh_trampoline&) = delete;
        refresh_trampoline& operator=(refresh_trampoline&&) = delete;
        ~refresh_trampoline();

        void clear()
        {
            dead = true;
            on_refresh = nullptr;
        }
    };

    namespace detail
    {
        inline std::mutex& refresh_peers_mutex()
        {
            static std::mutex mutex;
            return mutex;
        }

        inline std::unordered_map<std::uint64_t, std::weak_ptr<refresh_trampoline>>& refresh_peers()
        {
            static std::unordered_map<std::uint64_t, std::weak_ptr<refresh_trampoline>> peers;
            return peers;
        }

        [[nodiscard]] inline std::shared_ptr<refresh_trampoline> resolve_refresh_peer(jlong peer)
        {
            if (peer == 0)
            {
                return nullptr;
            }
            const std::scoped_lock lock(refresh_peers_mutex());
            const auto entry = refresh_peers().find(static_cast<std::uint64_t>(peer));
            return entry == refresh_peers().end() ? nullptr : entry->second.lock();
        }

        inline bool refresh_clear_pending(JNIEnv* env)
        {
            if (env->ExceptionCheck() == JNI_FALSE)
            {
                return false;
            }
            env->ExceptionDescribe(); // logcat breadcrumb, the channel the app host reads
            env->ExceptionClear();
            return true;
        }

        // The Java names this seam speaks. Every descriptor below was read out of `javap -s` against the
        // STAGED jar (~/.nuget/packages/xamarin.androidx.swiperefreshlayout/1.1.0.29), not assumed — the
        // dialog seam's header explains why that matters, and here it matters more: every lookup failure
        // in this seam degrades to "no spinner", which is indistinguishable from the bug it fixes.
        inline constexpr const char* k_refresh_bridge_class = "dev/mauicpp/MauiRefreshBridge";
        inline constexpr const char* k_swipe_refresh_class = "androidx/swiperefreshlayout/widget/SwipeRefreshLayout";
        inline constexpr const char* k_set_on_refresh_listener_sig =
            "(Landroidx/swiperefreshlayout/widget/SwipeRefreshLayout$OnRefreshListener;)V";

        inline void JNICALL native_on_refresh(JNIEnv* /*env*/, jclass /*bridge*/, jlong peer)
        {
            if (const std::shared_ptr<refresh_trampoline> state = resolve_refresh_peer(peer))
            {
                if (const std::function<void()> callback = state->on_refresh)
                {
                    callback(); // re-entrant (IsRefreshing=true runs the user's command): LAST, nothing after
                }
            }
        }

        // Idempotent, like the other two seams': RegisterNatives replaces an identical binding with
        // itself, so connecting handlers need no once-flag. False when the host does not carry the bridge
        // class — the caller then leaves the refresh host on its existing path rather than half-wiring a
        // SwipeRefreshLayout that spins forever and never fires the command.
        [[nodiscard]] inline bool register_refresh_natives(JNIEnv* env, jclass bridge_class)
        {
            // JNINativeMethod's name/signature are non-const char* and fnPtr a void* for historical
            // JNI-spec reasons — the casts are the API's own shape.
            static const std::array<JNINativeMethod, 1> k_methods{
                JNINativeMethod{.name = const_cast<char*>("nativeOnRefresh"),
                                .signature = const_cast<char*>("(J)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_on_refresh)},
            };
            if (env->RegisterNatives(bridge_class, k_methods.data(), static_cast<jint>(k_methods.size())) != JNI_OK)
            {
                refresh_clear_pending(env);
                return false;
            }
            return true;
        }
    } // namespace detail

    inline refresh_trampoline::~refresh_trampoline()
    {
        const std::scoped_lock lock(detail::refresh_peers_mutex());
        detail::refresh_peers().erase(id);
    }

    // Mint a registered peer. Register-on-construct / unregister-on-destruct means the registry can never
    // be left holding a key whose owner forgot to remove it.
    [[nodiscard]] inline std::shared_ptr<refresh_trampoline> make_refresh_peer()
    {
        auto peer = std::make_shared<refresh_trampoline>();
        const std::scoped_lock lock(detail::refresh_peers_mutex());
        detail::refresh_peers().insert_or_assign(peer->id, peer);
        return peer;
    }

    // `platformView.Refresh += OnSwipeRefresh` (RefreshViewHandler.Android.cs:21). False on ANY failure —
    // missing bridge class, failed RegisterNatives, missing setter, a throwing call — so the caller can
    // apply its all-or-nothing policy and fall back to a host with no spinner at all rather than to one
    // whose spinner never stops.
    [[nodiscard]] inline bool install_refresh_listener(JNIEnv* env, jobject swipe_refresh,
                                                       const refresh_trampoline* peer)
    {
        if (swipe_refresh == nullptr || peer == nullptr)
        {
            return false;
        }
        auto& cache = default_jni_cache();
        jclass bridge_class = cache.find_class(env, detail::k_refresh_bridge_class);
        jmethodID bridge_ctor = cache.method(env, detail::k_refresh_bridge_class, "<init>", "(J)V");
        jmethodID set_listener = cache.method(env, detail::k_swipe_refresh_class, "setOnRefreshListener",
                                              detail::k_set_on_refresh_listener_sig);
        if (bridge_class == nullptr || bridge_ctor == nullptr || set_listener == nullptr)
        {
            detail::refresh_clear_pending(env);
            return false;
        }
        if (!detail::register_refresh_natives(env, bridge_class))
        {
            return false;
        }
        // The bridge needs no C++-side pin: setOnRefreshListener stores a strong Java reference to it, so
        // it lives exactly as long as the SwipeRefreshLayout does.
        const local_ref<jobject> bridge{env, env->NewObject(bridge_class, bridge_ctor, static_cast<jlong>(peer->id))};
        if (detail::refresh_clear_pending(env) || !bridge)
        {
            return false;
        }
        env->CallVoidMethod(swipe_refresh, set_listener, bridge.get());
        return !detail::refresh_clear_pending(env);
    }

    // Teardown, in the order the other two seams document: clear the callbacks first (so a pull already in
    // flight is already a no-op), then drop the peer, which unregisters it — after which no NEW callback
    // can resolve it at all. The Java listener itself needs no uninstall: it dies with the
    // SwipeRefreshLayout, whose global ref the platform destructor releases in the same breath.
    inline void release_refresh_seam(std::shared_ptr<refresh_trampoline>& peer)
    {
        if (peer)
        {
            peer->clear();
        }
        peer.reset(); // unregisters, unless a callback is mid-flight holding the last ref
    }
} // namespace maui::platform::android
