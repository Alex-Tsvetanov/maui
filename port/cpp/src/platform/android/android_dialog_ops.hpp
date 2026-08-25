#pragma once
// Shared Android (JNI) click + modal-dialog seam. Used by button_handler.cpp and
// image_button_handler.cpp for the click trampoline alone, and by the three dialog-bearing handlers —
// picker, date_picker and time_picker — for the modals as well. Include only from those android
// partials (it reaches the jni_cache / scoped_env / app_context seam). VM-less safe: every helper acquires a scoped_env
// and quietly returns when there is no JavaVM / no Context, exactly like the other android_*_ops headers.
//
// WHAT THIS PORTS
//   PickerHandler.Android.cs      ConnectHandler's `platformView.Click += OnClick`, OnClick's
//                                 single-choice dialog, OnDialogShown/OnDialogDismiss, ShowDialog/
//                                 DismissDialog.
//   MauiDatePicker.cs / MauiTimePicker.cs  Initialize's SetOnClickListener(this) -> OnClick ->
//                                 ShowPicker?.Invoke().
//   DatePickerHandler.Android.cs  CreateDatePickerDialog (android.app.DatePickerDialog + OnDateSet),
//                                 ShowPickerDialog/HidePickerDialog, DatePickerExtensions'
//                                 UpdateMinimumDate/UpdateMaximumDate (the DIALOG's DatePicker
//                                 Min/MaxDate, never the field).
//   TimePickerHandler.Android.cs  CreateTimePickerDialog (android.app.TimePickerDialog + OnTimeSet)
//                                 and Use24HourView.
//
// WHY ONE SHARED HEADER RATHER THAN A PER-HANDLER RECIPE
// JNI RegisterNatives binds per CLASS, and the backend learned that the hard way. button_handler.cpp and
// image_button_handler.cpp EACH bound `nativeOnClick` on dev.mauicpp.NativeOnClickListener, each with its
// own platform struct's raw address as the jlong peer — so whichever connected last owned the binding
// process-wide, and the other control's taps arrived at a trampoline that reinterpret_cast the peer to
// the WRONG struct type. Type confusion, live on any page carrying both controls. Both handlers now share
// this seam, NativeOnClickListener is deleted, and dev.mauicpp.MauiDialogBridge's five natives are bound
// in exactly one place and dispatch through a live-peer registry.
//
// LIFETIME (PROFILE §8; the discipline gesture_platform_manager.cpp already follows)
// A shown android.app.Dialog is owned by the window manager and outlives its handler unless the
// handler tears it down. Four rules make a late callback a no-op instead of a crash:
//   1. The jlong peer Java carries is NEVER dereferenced directly. It is looked up in the live-peer
//      registry below; a peer that was never registered, or whose owner has been torn down, resolves
//      to nullptr and the trampoline returns.
//   2. Resolving hands back a STRONG shared_ptr that the whole callback body runs under, so
//      re-entrant teardown (a date-selected handler that navigates away and destroys the page) cannot
//      free the trampoline object under the running callback.
//   3. The std::function is COPIED out of the peer before it is invoked, so a teardown that clears the
//      callbacks mid-call does not destroy the target that is executing.
//   4. Every callback body performs its re-entrant call (the one that can destroy the handler) LAST,
//      and the trampoline touches nothing after invoking it.
// Teardown (release_dialog_seam, called from BOTH on_disconnect_handler and the platform struct's
// destructor, so a handler dropped without a disconnect is covered too) runs in this order: uninstall
// the click listener, clear the callbacks, dismiss + release the dialog, drop the peer. Clearing
// before dismissing means the synchronous onDismiss a dismiss() can fire is already a no-op; dropping
// the peer last unregisters it, after which no new callback can resolve it at all.

#include <jni.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/date_time.hpp"

namespace maui::platform::android
{
    // Monotonic, process-wide, never reused. Wrapping would take 585 years at one peer per nanosecond.
    [[nodiscard]] inline std::uint64_t next_dialog_peer_id()
    {
        static std::atomic<std::uint64_t> next{1}; // 0 stays reserved for "no peer"
        return next.fetch_add(1, std::memory_order_relaxed);
    }

    // The trampoline target: one per connected handler, heap-allocated so a callback can hold it alive
    // independently of the handler. Only the callbacks a given handler needs are set; the rest stay
    // empty and their trampolines are no-ops.
    struct dialog_trampoline
    {
        std::function<void()> on_click;                                 // field tapped -> show the modal
        std::function<void(int year, int month0, int day)> on_date_set; // DatePickerDialog.OnDateSet
        std::function<void(int hour, int minute)> on_time_set;          // TimePickerDialog.OnTimeSet
        std::function<void(int row)> on_item_selected;                  // SetSingleChoiceItems row tap
        std::function<void()> on_dismiss;                               // OnDialogDismiss
        // Set by clear(): the owning handler has torn down. A callback that has ALREADY re-entered user
        // code (which can destroy that handler) reads this — through the strong ref it is holding — to
        // decide whether touching the handler again is still legal. See picker_handler.cpp's row commit,
        // the one callback that has work to do after its re-entrant call.
        bool dead = false;

        // The registry key, and the jlong Java actually carries. NOT the address: an address is unique
        // only among LIVE objects, so a recycled allocation lets a stale token resolve to a DIFFERENT
        // live peer and drive the wrong control. A monotonic id never repeats, so a stale token resolves
        // to nothing for the rest of the process.
        const std::uint64_t id = next_dialog_peer_id();

        dialog_trampoline() = default;
        dialog_trampoline(const dialog_trampoline&) = delete;
        dialog_trampoline(dialog_trampoline&&) = delete;
        dialog_trampoline& operator=(const dialog_trampoline&) = delete;
        dialog_trampoline& operator=(dialog_trampoline&&) = delete;
        ~dialog_trampoline();

        // Teardown step 2: after this, every trampoline that still resolves this peer is a no-op, and a
        // callback already in flight can see (via `dead`) that its handler is gone.
        void clear()
        {
            dead = true;
            on_click = nullptr;
            on_date_set = nullptr;
            on_time_set = nullptr;
            on_item_selected = nullptr;
            on_dismiss = nullptr;
        }
    };

    namespace detail
    {
        // ---- the live-peer registry (rule 1 + 2) ---------------------------------------------------
        // Keyed by the peer's monotonic ID, valued by weak_ptr. Two distinct failures are covered, and
        // the second is why the key is not the address:
        //   USE-AFTER-FREE — an entry whose owner is gone resolves to nullptr rather than to freed
        //   storage, which is precisely what a raw jlong peer cannot do.
        //   ALIASING — an ADDRESS is unique only among LIVE objects. Keyed by address, this sequence
        //   resolved successfully and drove the WRONG control: handler A's peer lives at 0xABC and Java
        //   holds token 0xABC; A disconnects and the storage is freed; handler B allocates a peer and
        //   the allocator hands back 0xABC (it may, and in practice often does); a late callback still
        //   carrying A's token finds B's LIVE weak_ptr and locks it. The weak_ptr stopped the crash but
        //   not the cross-talk — a dismissed picker's date landing in a different picker. IDs are
        //   monotonic and never reused, so a stale token resolves to nothing for the rest of the
        //   process, whatever the allocator does.
        // Costs nothing at the seam: the id IS the jlong, so there is no extra indirection to resolve.
        inline std::mutex& dialog_peers_mutex()
        {
            static std::mutex mutex;
            return mutex;
        }

        inline std::unordered_map<std::uint64_t, std::weak_ptr<dialog_trampoline>>& dialog_peers()
        {
            static std::unordered_map<std::uint64_t, std::weak_ptr<dialog_trampoline>> peers;
            return peers;
        }

        // Resolve a peer to a strong ref that the caller holds for its whole body. Null for a peer that
        // was never registered, or whose last owner has dropped it.
        [[nodiscard]] inline std::shared_ptr<dialog_trampoline> resolve_dialog_peer(jlong peer)
        {
            if (peer == 0)
            {
                return nullptr;
            }
            const std::scoped_lock lock(dialog_peers_mutex());
            const auto entry = dialog_peers().find(static_cast<std::uint64_t>(peer));
            return entry == dialog_peers().end() ? nullptr : entry->second.lock();
        }

        // Clears any pending Java exception; true when one was pending (call sites then skip the
        // read-back). Same contract as each handler partial's file-local clear_pending.
        inline bool dialog_clear_pending(JNIEnv* env)
        {
            if (env->ExceptionCheck() == JNI_FALSE)
            {
                return false;
            }
            env->ExceptionDescribe(); // logcat/stderr breadcrumb, the channel the test host reads
            env->ExceptionClear();
            return true;
        }

        // The Java class names / signatures this seam speaks.
        inline constexpr const char* k_dialog_bridge_class = "dev/mauicpp/MauiDialogBridge";
        inline constexpr const char* k_checked_item_factory_class = "dev/mauicpp/MauiCheckedItemInflaterFactory";
        inline constexpr const char* k_dialog_class = "android/app/Dialog";
        inline constexpr const char* k_date_dialog_class = "android/app/DatePickerDialog";
        inline constexpr const char* k_time_dialog_class = "android/app/TimePickerDialog";
        // PickerHandler.Android.cs builds its single-choice list on MaterialAlertDialogBuilder, not the
        // framework AlertDialog.Builder. The port used the framework one because Material lived in an AAR
        // this backend was said not to carry — stale: the AAR closure is staged and dexed in both app hosts
        // (probed in the shipped classes.dex). The INTERACTION was already identical; what differed was the
        // CHROME, and it measured 35.71% of differing pixels on picker/android once the dialog opened.
        //
        // EVERY DESCRIPTOR BELOW CHANGES WITH THIS, which is why it is not a one-line swap. Verified with
        // javap against the staged jar rather than assumed:
        //   MaterialAlertDialogBuilder extends androidx.appcompat.app.AlertDialog$Builder (NOT android.app),
        //   its fluent setters are overridden to return MaterialAlertDialogBuilder, and create() returns
        //   androidx.appcompat.app.AlertDialog. Covariant overrides do leave bridge methods with the
        //   androidx return type, but this seam names the concrete Material overrides — they are the ones
        //   javap lists as declared, so GetMethodID cannot miss them.
        // The DOWNSTREAM path is unaffected: androidx.appcompat.app.AlertDialog derives from
        // AppCompatDialog -> ComponentDialog -> android.app.Dialog, so show/dismiss/setOnDismissListener
        // still resolve through k_dialog_class by superclass walking.
        inline constexpr const char* k_alert_builder_class =
            "com/google/android/material/dialog/MaterialAlertDialogBuilder";
        inline constexpr const char* k_alert_builder_ret =
            "Lcom/google/android/material/dialog/MaterialAlertDialogBuilder;";
        inline constexpr const char* k_native_view_class = "android/view/View";

        // ---- the five trampolines (one native function per Java method, bound once) -----------------
        // Each is `inline`, so its address is the same in every TU that includes this header and
        // re-registration from a second handler rebinds the very same pointer.

        inline void JNICALL native_dialog_click(JNIEnv* /*env*/, jclass /*bridge*/, jlong peer)
        {
            if (const std::shared_ptr<dialog_trampoline> state = resolve_dialog_peer(peer))
            {
                if (const std::function<void()> callback = state->on_click)
                {
                    callback(); // last statement: may show a dialog and re-enter the handler
                }
            }
        }

        inline void JNICALL native_dialog_date_set(JNIEnv* /*env*/, jclass /*bridge*/, jlong peer, jint year,
                                                   jint month, jint day)
        {
            if (const std::shared_ptr<dialog_trampoline> state = resolve_dialog_peer(peer))
            {
                if (const std::function<void(int, int, int)> callback = state->on_date_set)
                {
                    callback(year, month, day); // last statement (rule 4): may destroy the handler
                }
            }
        }

        inline void JNICALL native_dialog_time_set(JNIEnv* /*env*/, jclass /*bridge*/, jlong peer, jint hour,
                                                   jint minute)
        {
            if (const std::shared_ptr<dialog_trampoline> state = resolve_dialog_peer(peer))
            {
                if (const std::function<void(int, int)> callback = state->on_time_set)
                {
                    callback(hour, minute);
                }
            }
        }

        inline void JNICALL native_dialog_item_selected(JNIEnv* /*env*/, jclass /*bridge*/, jlong peer, jint which)
        {
            if (which < 0)
            {
                return; // a DialogInterface.BUTTON_* constant (the Cancel button), not a row
            }
            if (const std::shared_ptr<dialog_trampoline> state = resolve_dialog_peer(peer))
            {
                if (const std::function<void(int)> callback = state->on_item_selected)
                {
                    callback(which);
                }
            }
        }

        inline void JNICALL native_dialog_dismiss(JNIEnv* /*env*/, jclass /*bridge*/, jlong peer)
        {
            if (const std::shared_ptr<dialog_trampoline> state = resolve_dialog_peer(peer))
            {
                if (const std::function<void()> callback = state->on_dismiss)
                {
                    callback();
                }
            }
        }

        // Binds all five. Idempotent (RegisterNatives replaces an identical binding with itself), so
        // connecting handlers need no once-flag coordination. False when the host does not carry the
        // bridge class — then the whole dialog seam degrades to "no listener installed", exactly like
        // the VM-less path.
        [[nodiscard]] inline bool register_dialog_natives(JNIEnv* env, jclass bridge_class)
        {
            // JNINativeMethod's name/signature are non-const char* and fnPtr is a void* for historical
            // JNI-spec reasons — the const_casts/reinterpret_casts are the API's own shape.
            static const std::array<JNINativeMethod, 5> k_methods{
                JNINativeMethod{.name = const_cast<char*>("nativeOnClick"),
                                .signature = const_cast<char*>("(J)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_dialog_click)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnDateSet"),
                                .signature = const_cast<char*>("(JIII)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_dialog_date_set)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnTimeSet"),
                                .signature = const_cast<char*>("(JII)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_dialog_time_set)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnItemSelected"),
                                .signature = const_cast<char*>("(JI)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_dialog_item_selected)},
                JNINativeMethod{.name = const_cast<char*>("nativeOnDismiss"),
                                .signature = const_cast<char*>("(J)V"),
                                .fnPtr = reinterpret_cast<void*>(&native_dialog_dismiss)},
            };
            if (env->RegisterNatives(bridge_class, k_methods.data(), static_cast<jint>(k_methods.size())) != JNI_OK)
            {
                dialog_clear_pending(env);
                return false;
            }
            return true;
        }

        // Ensures MaterialAlertDialogBuilder's single-choice row layout renders its radio-ring indicator
        // (see MauiCheckedItemInflaterFactory.java for the full mechanism). `builder_context` is
        // `builder.getContext()` — the ContextThemeWrapper the Builder mints for itself, NOT the raw
        // Activity context and NOT the eventual dialog's own context: AlertParams/CheckedItemAdapter
        // inflate item rows through THIS SPECIFIC wrapper's LayoutInflater, captured at Builder
        // construction time, before AppCompatDialog's onCreate() ever runs — so this must be called
        // right after `new MaterialAlertDialogBuilder(context)`, before `.create()`. Idempotent and
        // side-effect-free if the factory is already installed (LayoutInflater#setFactory2 throws
        // otherwise) or if MauiCheckedItemInflaterFactory.class didn't make it into this dex (degrades to
        // the framework's plain CheckedTextView — a chrome miss, not a crash).
        inline void install_checked_item_inflater_factory(JNIEnv* env, jobject builder_context)
        {
            if (builder_context == nullptr)
            {
                return;
            }
            auto& cache = default_jni_cache();
            jclass li_class = cache.find_class(env, "android/view/LayoutInflater");
            jmethodID li_from = cache.static_method(env, "android/view/LayoutInflater", "from",
                                                    "(Landroid/content/Context;)Landroid/view/LayoutInflater;");
            jmethodID get_factory2 = cache.method(env, "android/view/LayoutInflater", "getFactory2",
                                                  "()Landroid/view/LayoutInflater$Factory2;");
            jmethodID set_factory2 = cache.method(env, "android/view/LayoutInflater", "setFactory2",
                                                  "(Landroid/view/LayoutInflater$Factory2;)V");
            if (li_class == nullptr || li_from == nullptr || get_factory2 == nullptr || set_factory2 == nullptr)
            {
                return;
            }
            local_ref<jobject> inflater{env, env->CallStaticObjectMethod(li_class, li_from, builder_context)};
            if (dialog_clear_pending(env) || !inflater)
            {
                return;
            }
            local_ref<jobject> existing{env, env->CallObjectMethod(inflater.get(), get_factory2)};
            if (dialog_clear_pending(env) || existing)
            {
                return; // already set — nothing to do (or setFactory2 would throw)
            }
            jclass factory_class = cache.find_class(env, k_checked_item_factory_class);
            jmethodID factory_ctor = cache.method(env, k_checked_item_factory_class, "<init>", "()V");
            if (factory_class == nullptr || factory_ctor == nullptr)
            {
                return; // host-provided class missing (see MauiCheckedItemInflaterFactory.java)
            }
            local_ref<jobject> factory{env, env->NewObject(factory_class, factory_ctor)};
            if (dialog_clear_pending(env) || !factory)
            {
                return;
            }
            env->CallVoidMethod(inflater.get(), set_factory2, factory.get());
            dialog_clear_pending(env);
        }

        // new dev.mauicpp.MauiDialogBridge(peer) — the listener object the widget and the dialogs hold.
        [[nodiscard]] inline local_ref<jobject> new_bridge(JNIEnv* env, const dialog_trampoline* peer)
        {
            auto& cache = default_jni_cache();
            jclass bridge_class = cache.find_class(env, k_dialog_bridge_class);
            jmethodID ctor = cache.method(env, k_dialog_bridge_class, "<init>", "(J)V");
            if (bridge_class == nullptr || ctor == nullptr || !register_dialog_natives(env, bridge_class))
            {
                return {}; // host-provided class missing (see MauiDialogBridge.java) — degrade quietly
            }
            local_ref<jobject> bridge{env, env->NewObject(bridge_class, ctor, static_cast<jlong>(peer->id))};
            if (dialog_clear_pending(env))
            {
                return {};
            }
            return bridge;
        }

        // dialog.setOnDismissListener(bridge) + dialog.show(), then pin the dialog as a global ref the
        // handler releases in teardown. Consumes `dialog` (a local ref); null on any JNI failure.
        [[nodiscard]] inline void* show_and_pin(JNIEnv* env, const local_ref<jobject>& dialog, jobject bridge)
        {
            auto& cache = default_jni_cache();
            jmethodID set_dismiss = cache.method(env, k_dialog_class, "setOnDismissListener",
                                                 "(Landroid/content/DialogInterface$OnDismissListener;)V");
            jmethodID show = cache.method(env, k_dialog_class, "show", "()V");
            if (show == nullptr)
            {
                return nullptr;
            }
            if (set_dismiss != nullptr)
            {
                env->CallVoidMethod(dialog.get(), set_dismiss, bridge);
                dialog_clear_pending(env);
            }
            // Pin BEFORE show() so the failure path below has a ref to drop; the caller only learns the
            // pointer from the return value, so this is NOT what makes the caller's re-entrancy guard
            // work (nothing calls back into the bridge during show(): no OnShowListener is installed).
            jobject pinned = env->NewGlobalRef(dialog.get());
            env->CallVoidMethod(dialog.get(), show);
            if (dialog_clear_pending(env))
            {
                // show() threw (a dead Activity token is the realistic case). Drop the pin and report
                // failure, or the handler would hold a "showing" dialog that never appeared and would
                // refuse to open a new one until teardown.
                if (pinned != nullptr)
                {
                    env->DeleteGlobalRef(pinned);
                }
                return nullptr;
            }
            return pinned;
        }
    } // namespace detail

    inline dialog_trampoline::~dialog_trampoline()
    {
        const std::scoped_lock lock(detail::dialog_peers_mutex());
        detail::dialog_peers().erase(id);
    }

    // Mint a registered peer. Register-on-construct / unregister-on-destruct means the registry can
    // never be left holding a key the owner forgot to remove.
    [[nodiscard]] inline std::shared_ptr<dialog_trampoline> make_dialog_peer()
    {
        auto peer = std::make_shared<dialog_trampoline>();
        const std::scoped_lock lock(detail::dialog_peers_mutex());
        detail::dialog_peers().insert_or_assign(peer->id, peer);
        return peer;
    }

    // ---- pure conversions (no JNI: unit-testable off-device) ----------------------------------------

    // java.util.Calendar / android.widget.DatePicker months are 0-BASED; System.DateTime months are
    // 1-based. C#: `CreateDatePickerDialog(date.Year, date.Month - 1, date.Day)` and, coming back,
    // `e.Date` (which Xamarin has already converted back to a 1-based DateTime).
    [[nodiscard]] inline int to_dialog_month(const maui::core::date_time& value)
    {
        return static_cast<int>(value.month()) - 1;
    }

    [[nodiscard]] inline maui::core::date_time from_dialog_date(int year, int month0, int day)
    {
        return maui::core::date_time{year, static_cast<unsigned>(month0 + 1), static_cast<unsigned>(day)};
    }

    // DatePicker.setMinDate/setMaxDate speak Unix epoch milliseconds. C#'s DatePickerExtensions spells
    // the same thing as `value.ToUniversalTime().Subtract(DateTime.MinValue.AddYears(1969))
    // .TotalMilliseconds` — DateTime.MinValue.AddYears(1969) IS 1970-01-01, so this is epoch millis.
    [[nodiscard]] inline jlong to_epoch_millis(const maui::core::date_time& value)
    {
        const auto days = std::chrono::duration_cast<std::chrono::milliseconds>(value.days().time_since_epoch());
        return static_cast<jlong>((days + value.time_of_day()).count());
    }

    // The bounds C# pushes when MinimumDate/MaximumDate are null: DateTime.MinValue / DateTime.MaxValue
    // through the same formula. MinValue = 0001-01-01T00:00 = -62135596800000 ms; MaxValue =
    // 9999-12-31T23:59:59.9999999 = 253402300799999 ms (truncated to whole ms by the (long) cast).
    inline constexpr jlong k_date_min_value_millis = -62135596800000LL;
    inline constexpr jlong k_date_max_value_millis = 253402300799999LL;

    // TimePickerHandler.Android.cs's Use24HourView: an empty format is 12-hour; the "t" standard
    // specifier defers to the device setting; any custom format is 24-hour iff it contains "HH".
    // The comparison is ORDINAL/case-sensitive on purpose — "HH" is the .NET 24-hour specifier and
    // "hh" the 12-hour one (IsCustom24HourFormat's own comment says exactly this).
    [[nodiscard]] inline bool use_24_hour_view(std::string_view format, bool system_is_24_hour)
    {
        if (format.empty())
        {
            return false;
        }
        if (format == "t")
        {
            return system_is_24_hour;
        }
        return format.find("HH") != std::string_view::npos;
    }

    // android.text.format.DateFormat.is24HourFormat(Context) — the device setting behind the "t" case.
    [[nodiscard]] inline bool system_is_24_hour(JNIEnv* env, jobject context)
    {
        if (context == nullptr)
        {
            return false;
        }
        auto& cache = default_jni_cache();
        jclass date_format = cache.find_class(env, "android/text/format/DateFormat");
        jmethodID is_24 = cache.static_method(env, "android/text/format/DateFormat", "is24HourFormat",
                                              "(Landroid/content/Context;)Z");
        if (date_format == nullptr || is_24 == nullptr)
        {
            return false;
        }
        const jboolean result = env->CallStaticBooleanMethod(date_format, is_24, context);
        return !detail::dialog_clear_pending(env) && result == JNI_TRUE;
    }

    // ---- the click listener ------------------------------------------------------------------------

    // MauiDatePicker/MauiTimePicker.Initialize's SetOnClickListener(this) / PickerHandler's
    // `platformView.Click += OnClick`. No-op (and harmless) when the host lacks the bridge class.
    inline void install_dialog_click_listener(JNIEnv* env, jobject widget, const dialog_trampoline* peer)
    {
        if (widget == nullptr || peer == nullptr)
        {
            return;
        }
        jmethodID set_listener = default_jni_cache().method(env, detail::k_native_view_class, "setOnClickListener",
                                                            "(Landroid/view/View$OnClickListener;)V");
        if (set_listener == nullptr)
        {
            return;
        }
        const local_ref<jobject> bridge = detail::new_bridge(env, peer);
        if (!bridge)
        {
            return;
        }
        env->CallVoidMethod(widget, set_listener, bridge.get());
        detail::dialog_clear_pending(env);
    }

    // DisconnectHandler's `platformView.Click -= OnClick`: the View drops its strong ref to the bridge,
    // so no NEW tap can reach the peer.
    inline void uninstall_dialog_click_listener(JNIEnv* env, jobject widget)
    {
        if (widget == nullptr)
        {
            return;
        }
        jmethodID set_listener = default_jni_cache().method(env, detail::k_native_view_class, "setOnClickListener",
                                                            "(Landroid/view/View$OnClickListener;)V");
        if (set_listener == nullptr)
        {
            return;
        }
        env->CallVoidMethod(widget, set_listener, static_cast<jobject>(nullptr));
        detail::dialog_clear_pending(env);
    }

    // ---- the three modals --------------------------------------------------------------------------
    // Each returns a GLOBAL ref to the shown android.app.Dialog (the caller stores it and releases it in
    // teardown), or nullptr when there is no VM / no Context / the JNI walk failed.

    // DatePickerHandler.CreateDatePickerDialog + ShowPickerDialog's min/max application: the
    // constraints go on the DIALOG's DatePicker (DatePickerExtensions), never on the field.
    [[nodiscard]] inline void* show_date_dialog(jobject context, const dialog_trampoline* peer, int year, int month0,
                                                int day, jlong min_millis, jlong max_millis)
    {
        const scoped_env env;
        if (!env || context == nullptr || peer == nullptr)
        {
            return nullptr;
        }
        auto& cache = default_jni_cache();
        jclass dialog_class = cache.find_class(env.get(), detail::k_date_dialog_class);
        jmethodID ctor =
            cache.method(env.get(), detail::k_date_dialog_class, "<init>",
                         "(Landroid/content/Context;Landroid/app/DatePickerDialog$OnDateSetListener;III)V");
        if (dialog_class == nullptr || ctor == nullptr)
        {
            return nullptr;
        }
        const local_ref<jobject> bridge = detail::new_bridge(env.get(), peer);
        if (!bridge)
        {
            return nullptr;
        }
        const local_ref<jobject> dialog{env.get(),
                                        env->NewObject(dialog_class, ctor, context, bridge.get(), year, month0, day)};
        if (detail::dialog_clear_pending(env.get()) || !dialog)
        {
            return nullptr;
        }
        jmethodID get_date_picker =
            cache.method(env.get(), detail::k_date_dialog_class, "getDatePicker", "()Landroid/widget/DatePicker;");
        if (get_date_picker != nullptr)
        {
            const local_ref<jobject> picker{env.get(), env->CallObjectMethod(dialog.get(), get_date_picker)};
            jmethodID set_min = cache.method(env.get(), "android/widget/DatePicker", "setMinDate", "(J)V");
            jmethodID set_max = cache.method(env.get(), "android/widget/DatePicker", "setMaxDate", "(J)V");
            if (!detail::dialog_clear_pending(env.get()) && picker && set_min != nullptr && set_max != nullptr)
            {
                env->CallVoidMethod(picker.get(), set_min, min_millis);
                detail::dialog_clear_pending(env.get());
                env->CallVoidMethod(picker.get(), set_max, max_millis);
                detail::dialog_clear_pending(env.get());
            }
        }
        return detail::show_and_pin(env.get(), dialog, bridge.get());
    }

    // TimePickerHandler.CreateTimePickerDialog: new TimePickerDialog(Context, callback, hour, minute,
    // Use24HourView).
    [[nodiscard]] inline void* show_time_dialog(jobject context, const dialog_trampoline* peer, int hour, int minute,
                                                bool is_24_hour)
    {
        const scoped_env env;
        if (!env || context == nullptr || peer == nullptr)
        {
            return nullptr;
        }
        auto& cache = default_jni_cache();
        jclass dialog_class = cache.find_class(env.get(), detail::k_time_dialog_class);
        jmethodID ctor =
            cache.method(env.get(), detail::k_time_dialog_class, "<init>",
                         "(Landroid/content/Context;Landroid/app/TimePickerDialog$OnTimeSetListener;IIZ)V");
        if (dialog_class == nullptr || ctor == nullptr)
        {
            return nullptr;
        }
        const local_ref<jobject> bridge = detail::new_bridge(env.get(), peer);
        if (!bridge)
        {
            return nullptr;
        }
        const local_ref<jobject> dialog{env.get(), env->NewObject(dialog_class, ctor, context, bridge.get(), hour,
                                                                  minute, is_24_hour ? JNI_TRUE : JNI_FALSE)};
        if (detail::dialog_clear_pending(env.get()) || !dialog)
        {
            return nullptr;
        }
        return detail::show_and_pin(env.get(), dialog, bridge.get());
    }

    // PickerHandler.OnClick's single-choice list, built on the REAL MaterialAlertDialogBuilder
    // (k_alert_builder_class, detail namespace above) — the claim that this APK-less backend falls back
    // to the framework android.app.AlertDialog.Builder is STALE; the AndroidX + Material AAR closure is
    // staged and dexed into both app hosts (tools/parity/lib/android-aar-lib.sh) and this function speaks
    // MaterialAlertDialogBuilder's real, covariant method signatures throughout. What WAS missing until
    // install_checked_item_inflater_factory (detail namespace) landed: the single-choice row's radio-ring
    // indicator, because this backend's host Activity is a plain android.app.Activity rather than
    // AppCompatActivity — see that helper's header comment for the mechanism, and
    // MauiCheckedItemInflaterFactory.java for the Java-side fix it installs.
    [[nodiscard]] inline void* show_items_dialog(jobject context, const dialog_trampoline* peer,
                                                 const std::vector<std::string>& items, std::string_view title,
                                                 int checked_item)
    {
        const scoped_env env;
        if (!env || context == nullptr || peer == nullptr)
        {
            return nullptr;
        }
        auto& cache = default_jni_cache();
        jclass builder_class = cache.find_class(env.get(), detail::k_alert_builder_class);
        jmethodID builder_ctor =
            cache.method(env.get(), detail::k_alert_builder_class, "<init>", "(Landroid/content/Context;)V");
        jclass char_sequence = cache.find_class(env.get(), "java/lang/CharSequence");
        if (builder_class == nullptr || builder_ctor == nullptr || char_sequence == nullptr)
        {
            return nullptr;
        }
        const local_ref<jobject> bridge = detail::new_bridge(env.get(), peer);
        if (!bridge)
        {
            return nullptr;
        }
        const local_ref<jobject> builder{env.get(), env->NewObject(builder_class, builder_ctor, context)};
        if (detail::dialog_clear_pending(env.get()) || !builder)
        {
            return nullptr;
        }
        // Must run BEFORE create(): AlertParams/CheckedItemAdapter capture the builder's OWN
        // ContextThemeWrapper's LayoutInflater at construction time, not at show() time — see
        // install_checked_item_inflater_factory's header comment for the full mechanism.
        if (jmethodID get_builder_context =
                cache.method(env.get(), detail::k_alert_builder_class, "getContext", "()Landroid/content/Context;"))
        {
            const local_ref<jobject> builder_context{env.get(),
                                                     env->CallObjectMethod(builder.get(), get_builder_context)};
            if (!detail::dialog_clear_pending(env.get()))
            {
                detail::install_checked_item_inflater_factory(env.get(), builder_context.get());
            }
        }
        // SetTitle(VirtualView.Title ?? string.Empty).
        if (jmethodID set_title = cache.method(
                env.get(), detail::k_alert_builder_class, "setTitle",
                "(Ljava/lang/CharSequence;)Lcom/google/android/material/dialog/MaterialAlertDialogBuilder;"))
        {
            const local_ref<jstring> text = to_jstring(env.get(), title);
            const local_ref<jobject> chained{env.get(), env->CallObjectMethod(builder.get(), set_title, text.get())};
            detail::dialog_clear_pending(env.get());
        }
        // SetSingleChoiceItems(items, SelectedIndex, rowTapped): a null item becomes "" in C# — the
        // port's items are std::string, so the empty string is already the only "null".
        const local_ref<jobjectArray> array{
            env.get(), env->NewObjectArray(static_cast<jsize>(items.size()), char_sequence, nullptr)};
        if (detail::dialog_clear_pending(env.get()) || !array)
        {
            return nullptr;
        }
        for (std::size_t index = 0; index < items.size(); ++index)
        {
            const local_ref<jstring> text = to_jstring(env.get(), items[index]);
            env->SetObjectArrayElement(array.get(), static_cast<jsize>(index), text.get());
            detail::dialog_clear_pending(env.get());
        }
        jmethodID set_items =
            cache.method(env.get(), detail::k_alert_builder_class, "setSingleChoiceItems",
                         "([Ljava/lang/CharSequence;ILandroid/content/DialogInterface$OnClickListener;)"
                         "Lcom/google/android/material/dialog/MaterialAlertDialogBuilder;");
        if (set_items == nullptr)
        {
            return nullptr;
        }
        {
            const local_ref<jobject> chained{env.get(),
                                             env->CallObjectMethod(builder.get(), set_items, array.get(),
                                                                   static_cast<jint>(checked_item), bridge.get())};
            if (detail::dialog_clear_pending(env.get()))
            {
                return nullptr;
            }
        }
        // SetNegativeButton(Android.Resource.String.Cancel, no-op): the framework string id, read as a
        // static field so no literal id is baked in. A null listener IS the C# empty lambda.
        jclass string_ids = cache.find_class(env.get(), "android/R$string");
        jmethodID set_negative = cache.method(env.get(), detail::k_alert_builder_class, "setNegativeButton",
                                              "(ILandroid/content/DialogInterface$OnClickListener;)Lcom/google/android/"
                                              "material/dialog/MaterialAlertDialogBuilder;");
        if (string_ids != nullptr && set_negative != nullptr)
        {
            const jfieldID cancel_field = env->GetStaticFieldID(string_ids, "cancel", "I");
            if (!detail::dialog_clear_pending(env.get()) && cancel_field != nullptr)
            {
                const jint cancel_id = env->GetStaticIntField(string_ids, cancel_field);
                const local_ref<jobject> chained{
                    env.get(),
                    env->CallObjectMethod(builder.get(), set_negative, cancel_id, static_cast<jobject>(nullptr))};
                detail::dialog_clear_pending(env.get());
            }
        }
        jmethodID create =
            cache.method(env.get(), detail::k_alert_builder_class, "create", "()Landroidx/appcompat/app/AlertDialog;");
        if (create == nullptr)
        {
            return nullptr;
        }
        const local_ref<jobject> dialog{env.get(), env->CallObjectMethod(builder.get(), create)};
        if (detail::dialog_clear_pending(env.get()) || !dialog)
        {
            return nullptr;
        }
        // _dialog.SetCanceledOnTouchOutside(true).
        if (jmethodID set_cancel_outside =
                cache.method(env.get(), detail::k_dialog_class, "setCanceledOnTouchOutside", "(Z)V"))
        {
            env->CallVoidMethod(dialog.get(), set_cancel_outside, JNI_TRUE);
            detail::dialog_clear_pending(env.get());
        }
        return detail::show_and_pin(env.get(), dialog, bridge.get());
    }

    // Drop the pin on a dialog that has ALREADY gone away (the OnDismiss path: a DatePickerDialog
    // dismisses itself once OK is tapped, and the framework AlertDialog on a Cancel/outside tap). Nulls
    // the caller's slot. Safe with nullptr, twice, and from any thread (scoped_env attaches on demand).
    inline void release_dialog(void*& dialog)
    {
        if (dialog == nullptr)
        {
            return;
        }
        auto* native = static_cast<jobject>(dialog);
        dialog = nullptr;
        const scoped_env env;
        if (env)
        {
            env->DeleteGlobalRef(native);
        }
    }

    // Dismiss + release a still-showing dialog (HidePickerDialog / DismissDialog / teardown). The slot
    // is nulled BEFORE dismiss() because dismiss() can synchronously re-enter through onDismiss.
    inline void dismiss_dialog(void*& dialog)
    {
        if (dialog == nullptr)
        {
            return;
        }
        auto* native = static_cast<jobject>(dialog);
        dialog = nullptr;
        const scoped_env env;
        if (!env)
        {
            return;
        }
        if (jmethodID dismiss = default_jni_cache().method(env.get(), detail::k_dialog_class, "dismiss", "()V"))
        {
            env->CallVoidMethod(native, dismiss);
            detail::dialog_clear_pending(env.get());
        }
        env->DeleteGlobalRef(native);
    }

    // The whole teardown, in the order the header documents. Called from BOTH on_disconnect_handler and
    // the platform struct's destructor — a handler destroyed without a disconnect must not leave a shown
    // dialog holding a bridge whose peer is gone.
    inline void release_dialog_seam(void* widget, void*& dialog, std::shared_ptr<dialog_trampoline>& peer)
    {
        if (widget != nullptr)
        {
            const scoped_env env;
            if (env)
            {
                uninstall_dialog_click_listener(env.get(), static_cast<jobject>(widget));
            }
        }
        if (peer)
        {
            peer->clear(); // a dismiss firing from the next line is already a no-op
        }
        dismiss_dialog(dialog);
        peer.reset(); // unregisters (unless a callback is mid-flight holding the last ref)
    }
} // namespace maui::platform::android
