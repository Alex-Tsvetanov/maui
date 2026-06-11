// The AppKit backend's platform ticker (see include/maui/animations/platform_ticker.hpp): an
// NSTimer on the main run loop in common modes, firing every 1000/max_fps ms.
//
// DEVIATION (documented): C# has no AppKit ticker — macOS apps run Mac Catalyst (UIKit), whose
// PlatformTicker.iOS.cs uses CADisplayLink. AppKit only gained a per-view CADisplayLink in macOS 14,
// and the standalone CVDisplayLink is deprecated (macOS 15) and fires on a background thread (which
// would need re-marshalling onto the UI thread the port's animation engine assumes, PROFILE §8). The
// NSTimer keeps the C# base Ticker's timer semantics — same cadence contract, main-thread delivery —
// which is what AppKit affords. The ios backend has the faithful CADisplayLink twin.
#import <Foundation/Foundation.h>

#include "maui/animations/platform_ticker.hpp"

#include <memory>

#include "maui/animations/ticker.hpp"

namespace maui::animations
{
    namespace
    {
        class appkit_ticker final : public ticker
        {
        public:
            explicit appkit_ticker(maui::core::i_dispatcher& dispatcher) : ticker(dispatcher)
            {
            }
            appkit_ticker(const appkit_ticker&) = delete;
            appkit_ticker& operator=(const appkit_ticker&) = delete;
            appkit_ticker(appkit_ticker&&) = delete;
            appkit_ticker& operator=(appkit_ticker&&) = delete;
            ~appkit_ticker() override
            {
                // Invalidate directly (not via the virtual stop()) — destructor-safe teardown that
                // also guarantees the timer block's captured `this` can never fire afterwards.
                [timer_ invalidate];
                timer_ = nil;
            }

            [[nodiscard]] bool is_running() const override
            {
                return timer_ != nil;
            }

            void start() override
            {
                if (timer_ != nil)
                {
                    return;
                }
                const NSTimeInterval interval = 1.0 / static_cast<NSTimeInterval>(max_fps());
                appkit_ticker* owner = this; // the timer never outlives the ticker (invalidated above)
                timer_ = [NSTimer timerWithTimeInterval:interval
                                                repeats:YES
                                                  block:^(NSTimer* _Nonnull) {
                                                    owner->invoke_fire();
                                                  }];
                [[NSRunLoop mainRunLoop] addTimer:timer_ forMode:NSRunLoopCommonModes];
            }

            void stop() override
            {
                if (timer_ == nil)
                {
                    return;
                }
                [timer_ invalidate];
                timer_ = nil;
            }

        private:
            NSTimer* timer_ = nil; // __strong under ARC
        };
    } // namespace

    std::shared_ptr<ticker> create_platform_ticker(maui::core::i_dispatcher& dispatcher)
    {
        return std::make_shared<appkit_ticker>(dispatcher);
    }
} // namespace maui::animations
