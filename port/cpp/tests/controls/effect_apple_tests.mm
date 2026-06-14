// Apple (AppKit) backend smoke for the G3 effects unit: an effect attaching to a real native-handled
// view. A button + a real button_handler create an NSButton; a platform_effect (registered in the effect
// registry, resolved as a routing_effect's inner — the faithful MAUI shape, where RoutingEffect.Inner IS
// the platform-specific effect) attaches through the element's lifecycle and resolves its Control to the
// genuine NSButton. Run only for MAUI_BACKEND=apple. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string_view>

#include "maui/controls/button.hpp"
#include "maui/controls/effect.hpp"
#include "maui/controls/effect_collection.hpp"
#include "maui/controls/effect_registry.hpp"
#include "maui/controls/i_effect_control_provider.hpp"
#include "maui/controls/platform_effect.hpp"
#include "maui/controls/routing_effect.hpp"
#include "maui/core/button_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::effect;
    using maui::controls::i_effect_control_provider;
    using maui::controls::platform_effect;
    using maui::controls::routing_effect;
    using maui::core::button_handler;

    NSButton* native_button(const std::shared_ptr<button_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->native;
    }

    // The platform-specific effect (RoutingEffect.Inner's role on apple): records its lifecycle + the
    // OnElementPropertyChanged forwarding so the smoke can assert the seam end-to-end on a real NSButton.
    class recording_platform_effect : public platform_effect<NSView, NSView>
    {
    public:
        bool on_attached_called = false;
        bool on_detached_called = false;
        int element_property_changed_count = 0;

    protected:
        void on_attached() override
        {
            on_attached_called = true;
        }
        void on_detached() override
        {
            on_detached_called = true;
        }
        void on_element_property_changed(std::string_view name) override
        {
            (void)name;
            ++element_property_changed_count;
        }
    };

    // The routing_effect a developer adds (its ctor is protected — only subclasses construct it with an id).
    class my_routing_effect : public routing_effect
    {
    public:
        explicit my_routing_effect(std::string_view id) : routing_effect(id)
        {
        }
    };

    // A minimal provider — its presence is what lets the element's attach path proceed (the native
    // resolution is done by the platform effect's own send_attached, off the element's handler).
    class noop_provider : public i_effect_control_provider
    {
    public:
        void register_effect(effect& target) override
        {
            (void)target;
        }
    };

    class apple_effect : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
            // Register the platform effect under a resolution id so a routing_effect resolves it as Inner.
            maui::controls::register_effect("MauiTests.AppleSmoke", [] {
                return std::shared_ptr<effect>(std::make_shared<recording_platform_effect>());
            });
        }
    };

    TEST_F(apple_effect, attaches_to_a_real_nsbutton_and_resolves_control)
    {
        button control; // publisher first (§8)
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        NSButton* const view = native_button(handler);

        // Add a routing_effect; its Inner is the registered platform effect (resolved at construction).
        auto routing = std::make_shared<my_routing_effect>("MauiTests.AppleSmoke");
        auto* platform = dynamic_cast<recording_platform_effect*>(routing->inner());
        ASSERT_NE(platform, nullptr);

        noop_provider provider;
        control.set_effect_control_provider(&provider);
        control.effects().add(routing); // provider already set -> attaches immediately

        // RoutingEffect.SendAttached forwards to Inner without setting its OWN IsAttached (it never calls
        // base.SendAttached) — so the routing wrapper stays "not attached" while its inner is the one that
        // actually attaches. Assert the INNER platform effect, matching C#.
        EXPECT_FALSE(routing->is_attached());
        EXPECT_TRUE(platform->is_attached()); // the inner platform effect attached
        EXPECT_TRUE(platform->on_attached_called);

        // The platform effect's Control resolved to the GENUINE NSButton (native_view of the real handler).
        EXPECT_EQ(platform->control(), view);
        EXPECT_EQ(platform->native_control(), (__bridge void*)view);
        // Container falls back to the native view here (a button has no separate container view).
        EXPECT_EQ(platform->container(), view);

        // A property change on the control reaches the platform effect (Element.OnPropertyChanged fan-out ->
        // RoutingEffect.SendOnElementPropertyChanged -> PlatformEffect.OnElementPropertyChanged while attached).
        platform->element_property_changed_count = 0;
        control.set_text("tap me");
        EXPECT_GT(platform->element_property_changed_count, 0);

        // Detaching clears the platform effect's native Control/Container and fires OnDetached.
        control.effects().remove(routing.get());
        EXPECT_TRUE(platform->on_detached_called);
        EXPECT_FALSE(platform->is_attached());
        EXPECT_EQ(platform->native_control(), nullptr);
        EXPECT_EQ(platform->native_container(), nullptr);
    }
} // namespace
