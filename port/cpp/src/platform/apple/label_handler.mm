// label_handler — Apple (AppKit / macOS) platform recipe: a non-editable, label-style NSTextField.
// Translated from LabelHandler.iOS.cs (UIKit's MauiLabel → AppKit's NSTextField label). Compiled as
// Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSTextField* as_label(void* native)
    {
        return (__bridge NSTextField*)native;
    }

    NSTextAlignment to_ns_text_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return NSTextAlignmentCenter;
            case maui::core::text_alignment::end:
                return NSTextAlignmentRight;
            case maui::core::text_alignment::justify:
                return NSTextAlignmentJustified;
            case maui::core::text_alignment::start:
                return NSTextAlignmentLeft;
        }
        return NSTextAlignmentLeft;
    }
} // namespace

namespace maui::core
{
    label_platform::~label_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native);
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void label_platform::update_visibility(maui::core::visibility value)
    {
        as_label(native).hidden = value != maui::core::visibility::visible;
    }

    void label_platform::update_opacity(double value)
    {
        as_label(native).alphaValue = value;
    }

    void label_platform::update_is_enabled(bool value)
    {
        [as_label(native) setEnabled:static_cast<BOOL>(value)];
    }

    void label_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_label(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<label_platform> label_handler::create_platform_view()
    {
        auto platform = std::make_unique<label_platform>();
        NSTextField* const field = [NSTextField labelWithString:@""]; // non-editable, borderless label style
        platform->native = (__bridge_retained void*)field;
        return platform;
    }

    void label_handler::map_text(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        as_label(platform->native).stringValue = value != nil ? value : @"";
    }

    void label_handler::map_text_color(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_label(platform->native).textColor = maui::platform::apple::to_ns_color(view.text_color());
        }
    }

    void label_handler::map_font(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_label(platform->native).font = maui::platform::apple::to_ns_font(view.font());
        }
    }

    void label_handler::map_horizontal_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_label(platform->native).alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    maui::graphics::size label_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_label(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void label_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_label(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
