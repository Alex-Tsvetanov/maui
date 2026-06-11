// main_thread - iOS (UIKit) platform partial: a FACADE over the existing GCD dispatcher
// (maui::core::gcd_dispatcher - one Obj-C++ definition shared with the apple backend), not a
// duplicate of it. is_main_thread is the inverse of is_dispatch_required() and begin_invoke
// posts through dispatch() onto the main queue; the inline-when-already-on-main gate lives in
// the shared facade (main_thread.hpp). Compiled as Objective-C++ with ARC for the ios backend.

#include <memory>
#include <utility>

#include "maui/core/gcd_dispatcher.hpp"
#include "maui/essentials/main_thread.hpp"

namespace maui::application_model
{
    namespace
    {
        class ios_main_thread final : public i_main_thread
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
            return std::make_shared<ios_main_thread>();
        }
    } // namespace detail
} // namespace maui::application_model
