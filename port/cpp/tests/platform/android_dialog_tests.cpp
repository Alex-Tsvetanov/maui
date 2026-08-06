// Android picker/date_picker/time_picker MODAL-DIALOG seam tests — ON the emulator inside the
// app_process widget test host (tools/android-testhost-run.sh).
//
// The defect these characterize: all three handlers used to install NO click listener and build NO
// dialog. A tap on the field produced nothing but the EditText's own press-state underline, while MAUI
// opens a modal (PickerHandler.Android.cs:23 `platformView.Click += OnClick`;
// MauiDatePicker.cs:42,54 OnClick -> ShowPicker + SetOnClickListener(this);
// DatePickerHandler.Android.cs:71-83 new DatePickerDialog; TimePickerHandler.Android.cs the same shape
// with TimePickerDialog).
//
// WHAT THIS FILE CAN AND CANNOT REACH IN THIS HOST
// The three widgets are EditText-based, and an android.widget.EditText CANNOT be constructed in the bare
// app_process test host: its TextView base calls setText() in the ctor -> AutofillManager -> a Settings
// ContentProvider the shell uid may not reach -> SecurityException (CMakeLists.txt documents this for
// editor/entry/switch/check_box/picker). A Dialog additionally needs a real Activity, which this host
// does not have either. So the tests below deliberately attach every handler through the CONTEXT-LESS
// path (`without_context` clears the process app_context for the duration, the documented VM-less
// degradation), which constructs no widget at all — and assert on the half that lives entirely in C++:
//   * the live-peer contract every trampoline routes through (a stale peer must be a no-op, not a
//     dereference of freed storage),
//   * the date/time <-> dialog argument conversions, 0-based month included,
//   * each handler's commit path from the dialog callback into the virtual view.
// NOT covered here, and verified only through the Android app host (a real Activity): the actual
// setOnClickListener install, the Dialog construction/show, and the row/OK/Cancel interaction. Those are
// stated as gaps rather than faked with a stub widget.

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <jni.h>

#include "android_dialog_ops.hpp"
#include "jni/app_context.hpp"
#include "maui/controls/date_picker.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/core/date_picker_handler.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/picker_handler.hpp"
#include "maui/core/time_picker_handler.hpp"
#include "testhost/test_host.hpp"

namespace
{
    using maui::core::date_time;
    using maui::core::time_span;
    using maui::platform::android::app_context;
    using maui::platform::android::dialog_trampoline;
    using maui::platform::android::make_dialog_peer;
    using maui::platform::android::set_app_context;

    // Attach the handlers with NO process Context, so create_platform_view takes its documented
    // context-less path and constructs no EditText (see the file header). Restores the host's Context on
    // the way out so the rest of the suite is unaffected.
    struct without_context
    {
        jobject saved = app_context();

        without_context()
        {
            set_app_context(nullptr);
        }
        ~without_context()
        {
            set_app_context(saved);
        }
        without_context(const without_context&) = delete;
        without_context(without_context&&) = delete;
        without_context& operator=(const without_context&) = delete;
        without_context& operator=(without_context&&) = delete;
    };

    // The control + attached handler, torn down in declaration order (the detach runs the handler's
    // on_disconnect_handler, then the platform struct dies) — the android_button_tests.cpp shape.
    template <typename Control, typename Handler> struct attached
    {
        without_context no_context{};
        Control control;
        std::shared_ptr<Handler> handler = std::make_shared<Handler>();

        attached()
        {
            control.set_handler(handler);
        }
        ~attached()
        {
            control.set_handler(nullptr);
        }
        attached(const attached&) = delete;
        attached(attached&&) = delete;
        attached& operator=(const attached&) = delete;
        attached& operator=(attached&&) = delete;

        // The trampoline peer the handler minted in on_connect_handler, as the jlong Java would carry.
        [[nodiscard]] jlong peer_token() const
        {
            auto* platform = handler->typed_platform_view();
            return platform == nullptr || !platform->dialog_peer
                       ? 0
                       : static_cast<jlong>(platform->dialog_peer->id);
        }

        [[nodiscard]] dialog_trampoline* peer() const
        {
            auto* platform = handler->typed_platform_view();
            return platform == nullptr ? nullptr : platform->dialog_peer.get();
        }
    };
} // namespace

// ---- the live-peer contract (shared by all three handlers) -----------------------------------------

// THE lifetime guarantee: the jlong Java carries is resolved through the registry before it is ever
// dereferenced, so a peer whose owner is gone is a no-op. Before this change the trampolines did not
// exist at all, and the recipe they would have copied (button_handler.cpp:622-629) reinterpret_casts the
// raw peer and calls straight through it — which on the freed peer below is a use-after-free.
TEST(AndroidDialogPeer, AStalePeerTokenIsANoOpNotADereference)
{
    int date_sets = 0;
    auto peer = make_dialog_peer();
    peer->on_date_set = [&date_sets](int, int, int) { ++date_sets; };
    const jlong token = static_cast<jlong>(peer->id);

    maui::platform::android::detail::native_dialog_date_set(nullptr, nullptr, token, 2020, 11, 31);
    EXPECT_EQ(date_sets, 1) << "a LIVE peer must still reach its callback";

    peer.reset(); // the owning handler tears down: the registry entry goes with it
    maui::platform::android::detail::native_dialog_date_set(nullptr, nullptr, token, 2021, 0, 1);
    EXPECT_EQ(date_sets, 1) << "a callback arriving after teardown must not run — and must not crash";
}

// A peer address that was never registered (a stray token, or one from another process generation) is
// equally inert, and so is the null token Java hands for a detached bridge.
TEST(AndroidDialogPeer, UnregisteredAndNullTokensResolveToNothing)
{
    dialog_trampoline never_registered;
    never_registered.on_click = [] { FAIL() << "an unregistered peer must never be resolved"; };

    maui::platform::android::detail::native_dialog_click(nullptr, nullptr,
                                                        static_cast<jlong>(never_registered.id));
    maui::platform::android::detail::native_dialog_click(nullptr, nullptr, 0);
    maui::platform::android::detail::native_dialog_dismiss(nullptr, nullptr, 0);
    maui::platform::android::detail::native_dialog_time_set(nullptr, nullptr, 0, 1, 2);
    maui::platform::android::detail::native_dialog_item_selected(nullptr, nullptr, 0, 3);
    SUCCEED();
}

// THE ALIASING GUARANTEE, and the reason the registry is keyed by a monotonic id rather than by the
// trampoline's address. An address is unique only among LIVE objects, so with an address key this exact
// sequence resolved SUCCESSFULLY and drove the wrong control: peer A is freed, peer B is allocated, the
// allocator hands back A's address, and a late callback carrying A's token locks B's live weak_ptr. The
// weak_ptr stopped the crash but not the cross-talk. A single-peer test cannot reach this — it needs a
// second peer to collide with, which is why the bug survived the rest of this file.
TEST(AndroidDialogPeer, ARecycledAddressDoesNotResurrectAStaleToken)
{
    jlong stale = 0;
    const void* first_address = nullptr;
    {
        auto first = make_dialog_peer();
        first->on_click = [] { FAIL() << "the FIRST peer is gone; its token must never resolve again"; };
        stale = static_cast<jlong>(first->id);
        first_address = first.get();
    } // first is destroyed and unregistered here

    // Allocate a fresh peer; the allocator is free to hand back the address just released, and commonly
    // does. Whether it happens to on this run is not the point — the assertion below holds either way,
    // and the address comparison is reported so a run where they DID collide is visible in the log.
    auto second = make_dialog_peer();
    int second_clicks = 0;
    second->on_click = [&second_clicks] { ++second_clicks; };
    if (static_cast<const void*>(second.get()) == first_address)
    {
        RecordProperty("addresses_collided", "yes"); // the case that used to mis-resolve
    }

    EXPECT_NE(stale, static_cast<jlong>(second->id)) << "ids must never be reused";
    maui::platform::android::detail::native_dialog_click(nullptr, nullptr, stale);
    EXPECT_EQ(second_clicks, 0) << "a stale token must not drive whatever now occupies that address";

    maui::platform::android::detail::native_dialog_click(nullptr, nullptr, static_cast<jlong>(second->id));
    EXPECT_EQ(second_clicks, 1) << "the live peer's own token must still work";
}

// clear() detaches without freeing: a callback already in flight can see `dead` and stop touching the
// handler (picker_handler.cpp's row commit does exactly this between the commit and the dismiss).
TEST(AndroidDialogPeer, ClearDetachesTheCallbacksAndFlagsTheHandlerDead)
{
    auto peer = make_dialog_peer();
    int clicks = 0;
    peer->on_click = [&clicks] { ++clicks; };
    const jlong token = static_cast<jlong>(peer->id);

    peer->clear();
    EXPECT_TRUE(peer->dead);
    maui::platform::android::detail::native_dialog_click(nullptr, nullptr, token);
    EXPECT_EQ(clicks, 0) << "a cleared peer still resolves (it is alive) but must run nothing";
}

// The negative `which` values a DialogInterface.OnClickListener receives for BUTTON_NEGATIVE (-2) etc.
// are buttons, not rows — routing them into SelectedIndex would commit row -2.
TEST(AndroidDialogPeer, ButtonWhichValuesAreNotRowSelections)
{
    auto peer = make_dialog_peer();
    int rows = -1;
    peer->on_item_selected = [&rows](int row) { rows = row; };
    const jlong token = static_cast<jlong>(peer->id);

    maui::platform::android::detail::native_dialog_item_selected(nullptr, nullptr, token, -2); // Cancel
    EXPECT_EQ(rows, -1) << "DialogInterface.BUTTON_NEGATIVE must not be committed as a row";
    maui::platform::android::detail::native_dialog_item_selected(nullptr, nullptr, token, 2);
    EXPECT_EQ(rows, 2);
}

// ---- the Java <-> C++ binding itself ----------------------------------------------------------------
// Everything above calls the trampolines as plain C++ functions, which never touches the half that can
// silently fail closed: dev.mauicpp.MauiDialogBridge being in the dex, its (J)V ctor, and — the
// all-or-nothing one — RegisterNatives matching all five .signature strings against that class. One
// wrong descriptor makes RegisterNatives fail wholesale, new_bridge return empty, the listener install
// return silently, and every handler degrade back to EXACTLY the pre-change behaviour (no listener, no
// dialog) with no other test noticing. This needs no Activity and no widget — only the JavaVM the test
// host already has.

namespace
{
    // Call one of the bridge's Java listener methods; the arguments those methods ignore are passed null.
    void call_bridge(JNIEnv* env, jobject bridge, const char* name, const char* signature, auto... args)
    {
        jmethodID method =
            maui::platform::android::default_jni_cache().method(env, "dev/mauicpp/MauiDialogBridge", name, signature);
        ASSERT_NE(method, nullptr) << "dev.mauicpp.MauiDialogBridge." << name << signature << " not found";
        env->CallVoidMethod(bridge, method, args...);
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            ADD_FAILURE() << "pending Java exception from MauiDialogBridge." << name;
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }
} // namespace

// The whole round trip: FindClass -> RegisterNatives (all five) -> new MauiDialogBridge(peer) ->
// Java onClick(View)/onDateSet(DatePicker,int,int,int) -> the native trampoline -> the C++ callback.
// Before this change dev.mauicpp.MauiDialogBridge did not exist, so new_bridge returns empty here.
TEST(AndroidDialogBridge, JavaCallbacksReachTheirCppPeer)
{
    const maui::platform::android::scoped_env env;
    ASSERT_TRUE(static_cast<bool>(env)) << "the widget test host must have a JavaVM";

    int clicks = 0;
    int year = 0;
    int month = -1;
    int day = 0;
    auto peer = make_dialog_peer();
    peer->on_click = [&clicks] { ++clicks; };
    peer->on_date_set = [&](int y, int m, int d) {
        year = y;
        month = m;
        day = d;
    };

    const maui::platform::android::local_ref<jobject> bridge =
        maui::platform::android::detail::new_bridge(env.get(), peer.get());
    ASSERT_TRUE(static_cast<bool>(bridge))
        << "the bridge class must be in the host's dex and all five natives must bind — a single wrong "
           "JNI signature fails RegisterNatives wholesale and silently disables every dialog";

    call_bridge(env.get(), bridge.get(), "onClick", "(Landroid/view/View;)V", static_cast<jobject>(nullptr));
    EXPECT_EQ(clicks, 1) << "View.OnClickListener.onClick must reach on_click";

    call_bridge(env.get(), bridge.get(), "onDateSet", "(Landroid/widget/DatePicker;III)V",
                static_cast<jobject>(nullptr), static_cast<jint>(2020), static_cast<jint>(11), static_cast<jint>(31));
    EXPECT_EQ(year, 2020);
    EXPECT_EQ(month, 11) << "Java hands the month through 0-based; the conversion belongs to the handler";
    EXPECT_EQ(day, 31);
}

// ---- the dialog argument conversions ---------------------------------------------------------------

// java.util.Calendar months are 0-based and System.DateTime months are 1-based; C# spells the outbound
// half `date.Value.Month - 1` and gets the inbound half back as a DateTime. Both directions are asserted
// because an off-by-one that is applied twice cancels out and hides.
TEST(AndroidDialogArgs, DialogMonthsAreZeroBasedInBothDirections)
{
    EXPECT_EQ(maui::platform::android::to_dialog_month(date_time{2020, 12, 31}), 11);
    EXPECT_EQ(maui::platform::android::to_dialog_month(date_time{2020, 1, 1}), 0);

    EXPECT_EQ(maui::platform::android::from_dialog_date(2020, 11, 31), (date_time{2020, 12, 31}));
    EXPECT_EQ(maui::platform::android::from_dialog_date(2020, 0, 1), (date_time{2020, 1, 1}));
}

// DatePicker.setMinDate/setMaxDate speak Unix epoch millis; C# spells it
// `value.ToUniversalTime().Subtract(DateTime.MinValue.AddYears(1969)).TotalMilliseconds`, and
// DateTime.MinValue.AddYears(1969) IS 1970-01-01. The two constants are the DateTime.MinValue /
// DateTime.MaxValue bounds C# pushes when MinimumDate/MaximumDate are null.
TEST(AndroidDialogArgs, EpochMillisMatchTheCSharpFormula)
{
    EXPECT_EQ(maui::platform::android::to_epoch_millis(date_time{1970, 1, 1}), 0);
    EXPECT_EQ(maui::platform::android::to_epoch_millis(date_time{2020, 12, 31}), 1609372800000LL);
    EXPECT_EQ(maui::platform::android::to_epoch_millis(date_time{1, 1, 1}),
              maui::platform::android::k_date_min_value_millis);
    // DateTime.MaxValue is the END of 9999-12-31 (23:59:59.999 after the (long) truncation).
    EXPECT_EQ(maui::platform::android::to_epoch_millis(date_time{9999, 12, 31}) + 86399999LL,
              maui::platform::android::k_date_max_value_millis);
}

// TimePickerHandler.Android.cs's Use24HourView + IsCustom24HourFormat. The comparison is ORDINAL: "HH"
// is the .NET 24-hour specifier and "hh" the 12-hour one, so a case-insensitive search would flip every
// 12-hour picker on the board to a 24-hour wheel.
TEST(AndroidDialogArgs, Use24HourViewIsOrdinalAndDefersToTheDeviceForT)
{
    EXPECT_FALSE(maui::platform::android::use_24_hour_view("", true)) << "an empty Format is 12-hour";
    EXPECT_TRUE(maui::platform::android::use_24_hour_view("t", true)) << "\"t\" follows the device";
    EXPECT_FALSE(maui::platform::android::use_24_hour_view("t", false));
    EXPECT_TRUE(maui::platform::android::use_24_hour_view("HH:mm", false));
    EXPECT_FALSE(maui::platform::android::use_24_hour_view("hh:mm tt", true))
        << "\"hh\" is the 12-HOUR specifier — a case-insensitive contains would read it as 24-hour";
    EXPECT_FALSE(maui::platform::android::use_24_hour_view("H:mm", true))
        << "IsCustom24HourFormat tests for \"HH\", not for a single H";
}

// ---- one commit path per handler -------------------------------------------------------------------

// DatePickerHandler's CreateDatePickerDialog callback: `VirtualView.Date = e.Date`. Before this change
// the handler had no on_date_set channel at all (no listener, no dialog), so a tap committed nothing;
// and a naive port that forwarded Java's month straight through would land 2020-11-31 -> November.
TEST(AndroidDatePickerDialog, OnDateSetCommitsThroughTheHandlerWithAZeroBasedMonth)
{
    attached<maui::controls::date_picker, maui::core::date_picker_handler> seam;
    ASSERT_NE(seam.peer(), nullptr) << "connect must mint a trampoline peer even with no widget";
    ASSERT_TRUE(static_cast<bool>(seam.peer()->on_date_set)) << "the OnDateSet channel must be wired";

    seam.peer()->on_date_set(2020, 11, 31); // java: year 2020, month 11 (December), day 31

    ASSERT_TRUE(seam.control.date().has_value());
    EXPECT_EQ(*seam.control.date(), (date_time{2020, 12, 31}));
    // The commit runs through the same on_done -> set_date path the portable drives use, so the field's
    // display mirror is re-rendered by the normal mapper (the default Format is "d").
    EXPECT_EQ(seam.handler->typed_platform_view()->text, "12/31/2020");
}

// TimePickerHandler's onTimeSetCallback: `VirtualView.Time = new TimeSpan(args.HourOfDay, args.Minute,
// 0)` — the seconds are DROPPED. Before this change there was no on_time_set channel at all.
TEST(AndroidTimePickerDialog, OnTimeSetCommitsTheWheelValueWithSecondsDropped)
{
    attached<maui::controls::time_picker, maui::core::time_picker_handler> seam;
    ASSERT_NE(seam.peer(), nullptr);
    ASSERT_TRUE(static_cast<bool>(seam.peer()->on_time_set));
    seam.control.set_time(time_span{9, 30, 45}); // a seconds-bearing value the pick must replace

    seam.peer()->on_time_set(13, 45);

    ASSERT_TRUE(seam.control.time().has_value());
    EXPECT_EQ(*seam.control.time(), (time_span{13, 45, 0}));
}

// PickerHandler's SetSingleChoiceItems callback: `VirtualView.SelectedIndex = e.Which` + UpdatePicker.
// Before this change there was no on_item_selected channel at all, so a tap on the field opened nothing
// and no row could ever be committed.
TEST(AndroidPickerDialog, ARowTapCommitsTheSelectedIndex)
{
    attached<maui::controls::picker, maui::core::picker_handler> seam;
    seam.control.items().add("alpha");
    seam.control.items().add("beta");
    seam.control.items().add("gamma");
    ASSERT_NE(seam.peer(), nullptr);
    ASSERT_TRUE(static_cast<bool>(seam.peer()->on_item_selected));

    seam.peer()->on_item_selected(2);

    EXPECT_EQ(seam.control.selected_index(), 2);
    EXPECT_EQ(seam.handler->typed_platform_view()->text, "gamma");
}

// ---- teardown: the whole point of the registry ------------------------------------------------------

// Disconnecting must drop the peer, so a callback from a dialog that outlived its handler resolves to
// nothing. Before this change the handlers installed nothing to tear down; had they followed the
// platform-struct-as-peer recipe, this exact call would dereference the destroyed platform struct.
TEST(AndroidDialogSeam, DisconnectDropsThePeerSoALateCallbackIsInert)
{
    jlong token = 0;
    {
        attached<maui::controls::date_picker, maui::core::date_picker_handler> seam;
        token = seam.peer_token();
        ASSERT_NE(token, 0);
        seam.control.set_handler(nullptr); // DisconnectHandler -> release_dialog_seam

        auto* platform = seam.handler->typed_platform_view();
        EXPECT_TRUE(platform == nullptr || !platform->dialog_peer) << "disconnect must drop the peer";

        // A DatePicker is NEVER dateless — it defaults to today, exactly as MAUI's DatePicker.Date does
        // — so the original form of this check (`EXPECT_FALSE(date().has_value())`) could not fail for
        // the reason it named. It read a value that was there before any callback ran and reported the
        // seam as leaking; measured 2026-08-06, the peer-drop assertion above passes, the sibling
        // destructor test passes, and a baseline probe showed date() already set at this point.
        // What the test MEANT is that a late callback changes NOTHING, so compare against the value the
        // control actually holds, and pick a callback date that would be unmistakable if it landed.
        const auto before = seam.control.date();
        ASSERT_TRUE(before.has_value()) << "a DatePicker defaults to today; if that ever changes, this "
                                           "test needs a different baseline, not a different assertion";

        maui::platform::android::detail::native_dialog_click(nullptr, nullptr, token);
        maui::platform::android::detail::native_dialog_date_set(nullptr, nullptr, token, 1999, 0, 1);
        maui::platform::android::detail::native_dialog_dismiss(nullptr, nullptr, token);
        EXPECT_EQ(seam.control.date(), before)
            << "a post-disconnect callback must commit nothing — 1999-01-01 landing here would mean the "
               "peer outlived the disconnect";
    }
    // ... and again after the handler and its platform struct are gone entirely.
    maui::platform::android::detail::native_dialog_date_set(nullptr, nullptr, token, 1999, 0, 1);
    SUCCEED();
}

// The same guarantee for the other two handlers, driven through their own peers: a handler destroyed
// WITHOUT a disconnect (the destructor path) must also leave nothing resolvable.
TEST(AndroidDialogSeam, DestructionWithoutDisconnectAlsoDropsThePeer)
{
    jlong time_token = 0;
    jlong picker_token = 0;
    {
        attached<maui::controls::time_picker, maui::core::time_picker_handler> time_seam;
        attached<maui::controls::picker, maui::core::picker_handler> picker_seam;
        time_token = time_seam.peer_token();
        picker_token = picker_seam.peer_token();
        ASSERT_NE(time_token, 0);
        ASSERT_NE(picker_token, 0);
    }
    maui::platform::android::detail::native_dialog_time_set(nullptr, nullptr, time_token, 6, 15);
    maui::platform::android::detail::native_dialog_item_selected(nullptr, nullptr, picker_token, 1);
    maui::platform::android::detail::native_dialog_dismiss(nullptr, nullptr, time_token);
    SUCCEED();
}
