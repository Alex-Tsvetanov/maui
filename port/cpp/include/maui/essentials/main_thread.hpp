#pragma once
// maui::application_model::main_thread    <=  Microsoft.Maui.ApplicationModel.MainThread (static facade)
// maui::application_model::i_main_thread  <=  the MainThread platform-partial seam
//                                              (MainThread.{ios.tvos.watchos.macos,netstandard}.cs)
//
// Runs code on the application's main (UI) thread. The port FACADES the existing dispatcher
// rather than duplicating it: the apple/ios partial wraps maui::core::gcd_dispatcher
// (is_main_thread = !is_dispatch_required(), begin-invoke = dispatch() onto the main queue - the
// exact NSThread.isMainThread / NSRunLoop.Main pair the C# partial uses, behind one GCD seam).
// The C# partial-method split (PlatformIsMainThread / PlatformBeginInvokeOnMainThread) becomes
// the i_main_thread interface; the netstandard partial's MainThreadImplementation custom-backend
// slot IS this library's standard set_current seam.
//
// The shared gate is C#'s: BeginInvokeOnMainThread runs the action INLINE when already on the
// main thread, otherwise forwards to the platform begin-invoke. The Task-shaped
// InvokeOnMainThreadAsync family collapses onto begin_invoke_on_main_thread (the port has no task
// type; completion callbacks would only re-wrap it).
//
// Backends: apple/macOS + ios REAL (the shared GCD dispatcher). Headless mirrors netstandard
// (both members throw until a custom implementation is installed - the SetCustomImplementation
// analog is configuring the headless fake).

#include <memory>
#include <utility>

#include "maui/core/move_only_function.hpp"

namespace maui::application_model
{
    using main_thread_action = maui::core::move_only_function<void()>;

    class i_main_thread
    {
    public:
        virtual ~i_main_thread() = default;

        // PlatformIsMainThread: true when the caller is on the UI thread.
        [[nodiscard]] virtual bool is_main_thread() const = 0;
        // PlatformBeginInvokeOnMainThread: post the action onto the UI thread (never inline).
        virtual void begin_invoke_on_main_thread(main_thread_action action) = 0;

    protected:
        i_main_thread() = default;
        i_main_thread(const i_main_thread&) = default;
        i_main_thread(i_main_thread&&) = default;
        i_main_thread& operator=(const i_main_thread&) = default;
        i_main_thread& operator=(i_main_thread&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory, one per backend under
        // src/platform/<backend>/essentials_main_thread.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_main_thread> make_main_thread();
    } // namespace detail

    // The static facade. begin_invoke_on_main_thread carries the C# shared gate (inline when
    // already on the main thread).
    class main_thread final
    {
    public:
        main_thread() = delete;

        [[nodiscard]] static bool is_main_thread()
        {
            return current().is_main_thread();
        }

        static void begin_invoke_on_main_thread(main_thread_action action)
        {
            if (is_main_thread())
            {
                action();
                return;
            }
            current().begin_invoke_on_main_thread(std::move(action));
        }

        // The lazy platform default + the test seam (the C# SetCustomImplementation/
        // ClearCustomImplementation pair; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_main_thread& current();
        static void set_current(std::shared_ptr<i_main_thread> implementation);
    };
} // namespace maui::application_model
