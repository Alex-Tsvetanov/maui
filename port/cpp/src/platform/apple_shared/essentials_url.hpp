#pragma once
// Shared NSString / NSURL conversion helpers for the Unit 27 comms .mm partials on BOTH Apple
// backends (the analog of having one set of helpers for the identical-on-AppKit/UIKit URL plumbing).
// to_ns_string uses [[NSString alloc] initWithBytes:...] (the codebase idiom: returns a non-null
// NSString for valid UTF-8, so the nullability analyzer is satisfied when it feeds a non-null
// parameter); to_ns_url returns a nullable NSURL the caller nil-checks before passing it on (the
// launcher's guarded pattern); get_native_url is the WebUtils.GetNativeUrl OriginalString->AbsoluteUri
// fallback the launchers route through. Header-only; included by src/platform/{apple,ios}/essentials_*.mm.

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

    namespace detail
    {
        // Minimal idempotent percent-encoder approximating System.Uri.AbsoluteUri: percent-encode
        // every byte outside the RFC 3986 unreserved + reserved set, leaving structural characters
        // (the gen-/sub-delims) and existing '%' triplets intact so re-normalizing an already-encoded
        // URI is a no-op. Bytes >= 0x80 (raw UTF-8) are encoded too, matching AbsoluteUri's escaping
        // of non-ASCII path/query bytes. This is not a full Uri parser - it only rescues a URI whose
        // raw form NSURL rejects (e.g. a literal space) by emitting its escaped equivalent.
        inline std::string normalize_uri(std::string_view value)
        {
            static constexpr char k_hex[] = "0123456789ABCDEF";
            std::string normalized;
            normalized.reserve(value.size());
            for (const char character : value)
            {
                const auto byte = static_cast<unsigned char>(character);
                const bool unreserved = (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') ||
                                        (byte >= '0' && byte <= '9') || byte == '-' || byte == '.' || byte == '_' ||
                                        byte == '~';
                // gen-delims + sub-delims + '%' (kept so existing %XX triplets are not double-encoded).
                const bool reserved = byte == ':' || byte == '/' || byte == '?' || byte == '#' || byte == '[' ||
                                      byte == ']' || byte == '@' || byte == '!' || byte == '$' || byte == '&' ||
                                      byte == '\'' || byte == '(' || byte == ')' || byte == '*' || byte == '+' ||
                                      byte == ',' || byte == ';' || byte == '=' || byte == '%';
                if (unreserved || reserved)
                {
                    normalized.push_back(character);
                }
                else
                {
                    normalized.push_back('%');
                    normalized.push_back(k_hex[byte >> 4U]);
                    normalized.push_back(k_hex[byte & 0x0FU]);
                }
            }
            return normalized;
        }
    } // namespace detail

    // The WebUtils.GetNativeUrl OriginalString->AbsoluteUri fallback: try the raw string first (the
    // analog of NSUrl(uri.OriginalString)); when NSURL cannot parse it, log the fallback and retry
    // with the normalized, percent-encoded form (the analog of uri.AbsoluteUri). Returns nil only
    // when neither form parses. Used by the apple/ios launcher partials.
    inline NSURL* get_native_url(std::string_view value)
    {
        if (NSURL* const direct = [NSURL URLWithString:to_ns_string(value)]; direct != nil)
        {
            return direct;
        }
        // Microsoft.Maui.WebUtils.GetNativeUrl: "Unable to create NSUrl from Original string, trying
        // Absolute URI". The C# logs the caught exception's message; NSURL returns nil rather than
        // throwing here, so there is no exception text to append.
        NSLog(@"Unable to create NSUrl from Original string, trying Absolute URI");
        return [NSURL URLWithString:to_ns_string(detail::normalize_uri(value))];
    }
} // namespace maui::platform::apple_shared
