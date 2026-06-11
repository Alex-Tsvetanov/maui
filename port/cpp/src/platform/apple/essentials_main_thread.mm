// main_thread - Apple (AppKit / macOS) platform partial: a FACADE over the existing GCD
// dispatcher (maui::core::gcd_dispatcher), not a duplicate of it. is_main_thread is the inverse
// of is_dispatch_required() (pthread_main_np - the supported stand-in for C#'s
// NSThread.Current.IsMainThread) and begin_invoke posts through dispatch() (the main queue -
// C#'s NSRunLoop.Main.BeginInvokeOnMainThread). The inline-when-already-on-main gate lives in
// the shared facade (main_thread.hpp). Compiled as Objective-C++ with ARC for the apple backend.

#include <memory>
#include <utility>

#include "maui/core/gcd_dispatcher.hpp"
#include "maui/essentials/main_thread.hpp"

namespace maui::application_model
{
    namespace
    {
        class apple_main_thread final : public i_main_thread
        {
        public:
            [[nodiscard]] bool is_main_thread() const override
            {
                return !dispatcher_.is_dispatch_required();
            }

            void begin_invoke_on_main_thread(main_thread_action action) override
            {
                dispatcher_.dispatch(std::move(action));
            }

        private:
            maui::core::gcd_dispatcher dispatcher_; // stateless (the main queue is a process global)
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_main_thread> make_main_thread()
        {
            return std::make_shared<apple_main_thread>();
        }
    } // namespace detail
} // namespace maui::application_model
