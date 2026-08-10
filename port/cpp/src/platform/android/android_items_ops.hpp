#pragma once
// Shared Android (JNI) RecyclerView items seam — the C++ half of dev.mauicpp.MauiItemsAdapter.
//
// WHAT THIS IS FOR
// MAUI's Android CarouselView is MauiCarouselRecyclerView (CarouselViewHandler.Android.cs:26-28), a
// RecyclerView subclass whose paging comes from SnapHelpers/SnapManager.cs attaching a PagerSnapHelper —
// NOT a ViewPager2, whatever collection_view_handler.cpp:75's deferral note says. A RecyclerView drives its
// content through an abstract Java class, and JNI cannot subclass one, so the adapter lives in Java
// (MauiItemsAdapter.java) and calls back here.
//
// WHY A SEPARATE HEADER FROM android_dialog_ops.hpp
// The two seams share a discipline but not a lifetime. A dialog callback fires ONCE, from a modal the user
// dismissed. An adapter callback fires REPEATEDLY on the UI thread for the whole of a fling, and
// RecyclerView caches the Views it was handed and may call back about a position whose data has already
// changed. Sharing one registry would let a torn-down picker and a live carousel contend for the same table
// for no benefit; keeping them apart also keeps each seam's invariants readable on its own.
//
// THE FOUR LIFETIME RULES ARE COPIED VERBATIM FROM THE DIALOG SEAM, because they were learned the hard way
// there (see android_dialog_ops.hpp's header for the incident):
//   1. the jlong Java carries is NEVER dereferenced — it is a monotonic id looked up in the registry below,
//      so a stale token resolves to nothing rather than to freed storage or, worse, to a RECYCLED address
//      that happens to hold a different live peer;
//   2. resolving hands back a STRONG shared_ptr held for the whole callback body, so a re-entrant teardown
//      cannot free the trampoline under the running callback;
//   3. the std::function is COPIED out before it is invoked, so a teardown that clears the callbacks
//      mid-call does not destroy the target that is executing;
//   4. every body performs its re-entrant call LAST and touches nothing afterwards.
//
// AND ONE RULE THE DIALOG SEAM DID NOT NEED: a bind that resolves no peer must be a SILENT NO-OP, not an
// error. RecyclerView will happily ask a torn-down adapter to bind — an empty cell renders blank, which is
// the correct degradation. The same is true VM-less: every entry point tolerates a null peer.
//
// A NOTE ON THE PEER-POINTER ALTERNATIVE, since the layout seam uses one. MauiLayout carries a RAW handler
// address (crossPlatformPeer), installed at on_connect_handler and cleared to 0 at on_disconnect_handler,
// and that is genuinely safe *for a layout*: the only caller is onLayout, which the parent drives
// synchronously while the view is attached. A RecyclerView adapter is not that — it is retained by the
// recycler, called from posted messages, and outlives individual holders. The registry costs one hash
// lookup per callback and removes an entire class of failure that a cleared-pointer scheme only avoids if
// the clear always wins the race. That is why this seam keeps the id.

#include <jni.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace maui::platform::android
{
    // Monotonic, process-wide, never reused — see rule 1. Shares nothing with the dialog seam's counter;
    // the two id spaces are independent because the two registries are.
    [[nodiscard]] inline std::uint64_t next_items_peer_id()
    {
        static std::atomic<std::uint64_t> next{1}; // 0 stays reserved for "no peer"
        return next.fetch_add(1, std::memory_order_relaxed);
    }

    // The trampoline target: one per connected items handler, heap-allocated so a callback can hold it
    // alive independently of the handler.
    struct items_adapter_trampoline
    {
        // RecyclerView.Adapter.getItemCount(). Returning 0 for a torn-down peer empties the list, which is
        // the correct degradation — the alternative (a stale count) makes RecyclerView bind positions whose
        // data is gone.
        std::function<int()> item_count;
        // onBindViewHolder: fill the holder's container (already emptied Java-side) with the realized cell
        // for `position`. The container is a LOCAL ref owned by RecyclerView — fill it and do not retain it.
        std::function<void(int position, jobject container)> bind_holder;
        // onViewRecycled: the cell left the screen and Java has dropped its children. Release whatever the
        // port realized for that position.
        std::function<void(int position)> recycle_holder;
        // The settled-page write-back (CarouselView.Position). Fired once per settle, never per frame.
        std::function<void(int position)> page_settled;

        // Set by clear(): the owning handler has torn down. A callback that has ALREADY re-entered user
        // code reads this — through the strong ref it holds — to decide whether touching the handler again
        // is still legal.
        bool dead = false;

        const std::uint64_t id = next_items_peer_id();

        items_adapter_trampoline() = default;
        items_adapter_trampoline(const items_adapter_trampoline&) = delete;
        items_adapter_trampoline(items_adapter_trampoline&&) = delete;
        items_adapter_trampoline& operator=(const items_adapter_trampoline&) = delete;
        items_adapter_trampoline& operator=(items_adapter_trampoline&&) = delete;
        ~items_adapter_trampoline();

        void clear()
        {
            dead = true;
            item_count = nullptr;
            bind_holder = nullptr;
            recycle_holder = nullptr;
            page_settled = nullptr;
        }
    };

    namespace detail
    {
        inline std::mutex& items_peers_mutex()
        {
            static std::mutex mutex;
            return mutex;
        }

        inline std::unordered_map<std::uint64_t, std::weak_ptr<items_adapter_trampoline>>& items_peers()
        {
            static std::unordered_map<std::uint64_t, std::weak_ptr<items_adapter_trampoline>> peers;
            return peers;
        }

        [[nodiscard]] inline std::shared_ptr<items_adapter_trampoline> resolve_items_peer(jlong peer)
        {
            if (peer == 0)
            {
                return nullptr;
            }
            const std::scoped_lock lock(items_peers_mutex());
            const auto entry = items_peers().find(static_cast<std::uint64_t>(peer));
            return entry == items_peers().end() ? nullptr : entry->second.lock();
        }

        inline bool items_clear_pending(JNIEnv* env)
        {
            if (env->ExceptionCheck() == JNI_FALSE)
            {
                return false;
            }
            env->ExceptionClear();
            return true;
        }

        // ---- the four trampolines -------------------------------------------------------------------
        [[nodiscard]] inline jint JNICALL native_items_count(JNIEnv* /*env*/, jclass /*adapter*/, jlong peer)
        {
            if (const std::shared_ptr<items_adapter_trampoline> state = resolve_items_peer(peer))
            {
                if (const std::function<int()> callback = state->item_count)
                {
                    return static_cast<jint>(callback());
                }
            }
            return 0; // torn down / unwired: an empty list, never a stale count
        }

        inline void JNICALL native_items_bind(JNIEnv* /*env*/, jclass /*adapter*/, jlong peer, jint position,
                                              jobject container)
        {
            if (position < 0 || container == nullptr)
            {
                return;
            }
            if (const std::shared_ptr<items_adapter_trampoline> state = resolve_items_peer(peer))
            {
                if (const std::function<void(int, jobject)> callback = state->bind_holder)
                {
                    callback(static_cast<int>(position), container);
                }
            }
        }

        inline void JNICALL native_items_recycle(JNIEnv* /*env*/, jclass /*adapter*/, jlong peer, jint position)
        {
            if (const std::shared_ptr<items_adapter_trampoline> state = resolve_items_peer(peer))
            {
                if (const std::function<void(int)> callback = state->recycle_holder)
                {
                    callback(static_cast<int>(position));
                }
            }
        }

        inline void JNICALL native_items_page_settled(JNIEnv* /*env*/, jclass /*adapter*/, jlong peer, jint position)
        {
            if (position < 0)
            {
                return; // RecyclerView.NO_POSITION for a holder that left the adapter mid-fling
            }
            if (const std::shared_ptr<items_adapter_trampoline> state = resolve_items_peer(peer))
            {
                if (const std::function<void(int)> callback = state->page_settled)
                {
                    callback(static_cast<int>(position)); // re-entrant: LAST, and nothing after it
                }
            }
        }

        inline constexpr const char* k_items_adapter_class = "dev/mauicpp/MauiItemsAdapter";

        // Idempotent, like the dialog seam's: RegisterNatives replaces an identical binding with itself, so
        // connecting handlers need no once-flag. False when the host does not carry the adapter class —
        // the caller then leaves the items host on its existing path rather than half-wiring a RecyclerView.
        [[nodiscard]] inline bool register_items_natives(JNIEnv* env, jclass adapter_class)
        {
            static const std::array<JNINativeMethod, 4> k_methods{
                JNINativeMethod{.name = const_cast<char*>("nativeGetItemCount"),
                                .signature = const_cast<char*>("(J)I"),
                                .fnPtr = reinterpret_cast<void*>(&native_items_count)},
                JNINativeMethod{.name = const_cast<char*>("nativeBindHolder"),
                                .signature = const_cast<char*>("(JILandroid/view/View;)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_items_bind)},
                JNINativeMethod{.name = const_cast<char*>("nativeRecycleHolder"),
                                .signature = const_cast<char*>("(JI)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_items_recycle)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnPageSettled"),
                                .signature = const_cast<char*>("(JI)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_items_page_settled)},
            };
            if (env->RegisterNatives(adapter_class, k_methods.data(), static_cast<jint>(k_methods.size())) != JNI_OK)
            {
                items_clear_pending(env);
                return false;
            }
            return true;
        }
    } // namespace detail

    // Register/unregister mirror the dialog seam exactly: a peer becomes resolvable only once its owner
    // holds it, and stops resolving the moment the owner drops it.
    inline void register_items_peer(const std::shared_ptr<items_adapter_trampoline>& state)
    {
        if (!state)
        {
            return;
        }
        const std::scoped_lock lock(detail::items_peers_mutex());
        detail::items_peers()[state->id] = state;
    }

    inline items_adapter_trampoline::~items_adapter_trampoline()
    {
        const std::scoped_lock lock(detail::items_peers_mutex());
        detail::items_peers().erase(id);
    }
} // namespace maui::platform::android
