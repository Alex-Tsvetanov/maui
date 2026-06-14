#pragma once
// Shared NSString / NSURL conversion helpers for the Unit 27 comms .mm partials on BOTH Apple
// backends (the analog of having one set of helpers for the identical-on-AppKit/UIKit URL plumbing).
// to_ns_string uses [[NSString alloc] initWithBytes:...] (the codebase idiom: returns a non-null
// NSString for valid UTF-8, so the nullability analyzer is satisfied when it feeds a non-null
// parameter); to_ns_url returns a nullable NSURL the caller nil-checks before passing it on (the
// launcher's guarded pattern). Header-only; included by src/platform/{apple,ios}/essentials_*.mm.

#import <Foundation/Foundation.h>

#include <string>
#include <string_view>

namespace maui::platform::apple_shared
{
    // A non-null NSString from UTF-8 bytes (the initWithBytes idiom shared with the user-defaults
    // helper). Messaging the result is always safe.
    inline NSString* to_ns_string(std::string_view value)
    {
        return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
    }

    // A (nullable) NSURL from a UTF-8 string; nil for a string NSURL cannot parse. Callers guard the
    // result with `if (url != nil)` before passing it to a non-null parameter.
    inline NSURL* to_ns_url(std::string_view value)
    {
        return [NSURL URLWithString:to_ns_string(value)];
    }

    // A std::string from an NSString (nil yields ""); messaging nil returns nullptr.
    inline std::string to_std_string(NSString* value)
    {
        const char* const utf8 = [value UTF8String];
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }
} // namespace maui::platform::apple_shared
