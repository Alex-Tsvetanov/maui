// The iOS backend's platform ticker (see include/maui/animations/platform_ticker.hpp): a
// CADisplayLink added to the current run loop in common modes — ported 1:1 from
// src/Core/src/Animations/PlatformTicker.iOS.cs (which also covers Mac Catalyst). CADisplayLink has
// no block API, so a small ObjC proxy target forwards the frame callback into the C++ ticker (the
// .NET binding's CADisplayLink.Create(Action) does the same internally).
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>

#include "maui/animations/platform_ticker.hpp"

#include <functional>
#include <memory>
#include <utility>

#include "maui/animations/ticker.hpp"

@interface MauiDisplayLinkProxy : NSObject
- (instancetype)initWithCallback:(std::function<void()>)callback;
- (void)tick:(CADisplayLink*)link;
@end

@implementation MauiDisplayLinkProxy
{
    std::function<void()> _callback;
}

- (instancetype)initWithCallback:(std::function<void()>)callback
{
    self = [super init];
    if (self != nil)
    {
        _callback = std::move(callback);
    }
    return self;
}

- (void)tick:(__unused CADisplayLink*)link
{
    if (_callback)
    {
        _callback();
    }
}
@end

namespace maui::animations
{
    namespace
    {
        class display_link_ticker final : public ticker
        {
        public:
            explicit display_link_ticker(maui::core::i_dispatcher& dispatcher) : ticker(dispatcher)
            {
            }
            display_link_ticker(const display_link_ticker&) = delete;
            display_link_ticker& operator=(const display_link_ticker&) = delete;
            display_link_ticker(display_link_ticker&&) = delete;
            display_link_ticker& operator=(display_link_ticker&&) = delete;
            ~display_link_ticker() override
            {
                // Invalidate directly (not via the virtual stop()): removes the link from every run
                // loop, so the proxy's captured `this` can never fire afterwards.
                [link_ invalidate];
                link_ = nil;
                proxy_ = nil;
            }

            // C# PlatformTicker.IsRunning => _link != null.
            [[nodiscard]] bool is_running() const override
            {
                return link_ != nil;
            }

            // C# PlatformTicker.Start: create the link and add it to the current run loop (common modes).
            void start() override
            {
                if (link_ != nil)
                {
                    return;
                }
                proxy_ = [[MauiDisplayLinkProxy alloc] initWithCallback:[this] { invoke_fire(); }];
                link_ = [CADisplayLink displayLinkWithTarget:proxy_ selector:@selector(tick:)];
                [link_ addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
            }

            // C# PlatformTicker.Stop: remove from the run loop and dispose the link.
            void stop() override
            {
                if (link_ == nil)
                {
                    return;
                }
                [link_ removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
                [link_ invalidate];
                link_ = nil;
                proxy_ = nil;
            }

        private:
            CADisplayLink* link_ = nil;         // __strong under ARC
            MauiDisplayLinkProxy* proxy_ = nil; // the link's target (the link retains it too)
        };
    } // namespace

    std::shared_ptr<ticker> create_platform_ticker(maui::core::i_dispatcher& dispatcher)
    {
        return std::make_shared<display_link_ticker>(dispatcher);
    }
} // namespace maui::animations
