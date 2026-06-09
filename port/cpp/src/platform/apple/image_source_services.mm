// Per-source image services + decode_image_bytes — APPLE (AppKit / macOS) backend. The real-native twin of
// src/platform/headless/image_source_services.cpp: each load produces an NSImage, retained into the
// image_source_result (with a CFRelease disposer — RAII, the loader drops the previous result on the next
// load). Compiled as Objective-C++ with ARC for the `apple` backend.
//
// file   — [[NSImage alloc] initWithContentsOfFile:] (the synchronous file path, as image_handler.mm uses).
// stream — NSData from the source's bytes → NSImage (the bytes are already in memory; decode is synchronous).
// uri    — file:// bytes are read cross-platform (read_uri_bytes) then decoded; http(s) is fetched via
//          NSData(contentsOfURL:) (synchronous this cut — a production stack would move this to a background
//          queue + marshal the apply back through the dispatcher; the loader's apply marshalling already
//          supports that). Translated from UriImageSourceService.iOS.cs / StreamImageSourceService.iOS.cs.
// font   — the glyph is drawn (an NSAttributedString in the source's font + color) into an NSImage sized to
//          the glyph. The result is RESOLUTION-DEPENDENT (C# FontImageSourceService.iOS passes true).
//          Translated from FontImageSourceService.iOS.cs (GetPlatformImage). DEVIATION: no IFontManager —
//          the source's font value maps to NSFont via apple_conversions (an unknown family → system font).

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include "apple_conversions.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/file_image_source_service.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/i_font_image_source.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/i_uri_image_source.hpp"
#include "maui/core/image_decode.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_bytes.hpp"
#include "maui/core/uri_image_source_service.hpp"

namespace
{
    // Wrap a (non-nil) NSImage into a loaded result: the void* slot retains one reference; the disposer
    // CFReleases it when the result is dropped. A nil image yields a !loaded() result. resolution_dependent
    // mirrors C# IImageSourceServiceResult.IsResolutionDependent (true for the font service).
    maui::core::image_source_result make_result(NSImage* image, std::string kind, std::string detail,
                                                bool resolution_dependent = false)
    {
        if (image == nil)
        {
            return {};
        }
        void* const retained = (__bridge_retained void*)image; // result owns one reference
        return maui::core::image_source_result{retained, [retained] { CFRelease(retained); }, std::move(kind),
                                               std::move(detail), resolution_dependent};
    }

    // Draw `glyph` in `font`/`color` into a new NSImage sized to the glyph's bounding box. Returns nil for
    // an empty glyph / zero-size text. Ports FontImageSourceService.iOS GetPlatformImage's draw-into-context.
    NSImage* image_from_glyph(std::string_view glyph, NSFont* font, NSColor* color)
    {
        const std::string text(glyph);
        NSString* const ns_glyph = [NSString stringWithUTF8String:text.c_str()];
        if (ns_glyph == nil || ns_glyph.length == 0 || font == nil)
        {
            return nil;
        }
        NSDictionary<NSAttributedStringKey, id>* const attrs = @{
            NSFontAttributeName : font,
            NSForegroundColorAttributeName : (color != nil ? color : NSColor.blackColor),
        };
        const NSSize size = [ns_glyph sizeWithAttributes:attrs];
        if (size.width <= 0 || size.height <= 0)
        {
            return nil;
        }
        NSImage* const image = [[NSImage alloc] initWithSize:size];
        [image lockFocus];
        [ns_glyph drawAtPoint:NSMakePoint(0, 0) withAttributes:attrs];
        [image unlockFocus];
        return image;
    }

    NSData* to_ns_data(const maui::core::image_bytes& bytes)
    {
        if (bytes.empty())
        {
            return nil;
        }
        return [NSData dataWithBytes:bytes.data() length:static_cast<NSUInteger>(bytes.size())];
    }

    NSImage* image_from_file(std::string_view path)
    {
        const std::string file(path);
        NSString* const ns_path = [NSString stringWithUTF8String:file.c_str()];
        if (ns_path == nil)
        {
            return nil;
        }
        return [[NSImage alloc] initWithContentsOfFile:ns_path];
    }
} // namespace

namespace maui::core
{
    // Apple decode: bytes → NSData → NSImage, retained into the result (nil image → !loaded()).
    image_source_result decode_image_bytes(const image_bytes& bytes, std::string kind, std::string detail)
    {
        NSData* const data = to_ns_data(bytes);
        if (data == nil)
        {
            return {};
        }
        NSImage* const image = [[NSImage alloc] initWithData:data];
        return make_result(image, std::move(kind), std::move(detail));
    }

    void file_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* file_src = dynamic_cast<const i_file_image_source*>(&source);
        if (file_src == nullptr || file_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        on_result(make_result(image_from_file(file_src->file()), "file", std::string(file_src->file())));
    }

    void uri_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                        completion on_result)
    {
        auto* uri_src = dynamic_cast<i_uri_image_source*>(&source);
        if (uri_src == nullptr || uri_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const std::string uri(uri_src->uri());

        // Local files go through the cross-platform reader (so `file://` matches the loader's fast-path);
        // an http(s) URI is fetched synchronously via NSData(contentsOfURL:) this cut.
        image_bytes bytes = read_uri_bytes(uri);
        if (bytes.empty() && (uri.starts_with("http://") || uri.starts_with("https://")))
        {
            NSString* const ns_uri = [NSString stringWithUTF8String:uri.c_str()];
            NSURL* const url = ns_uri != nil ? [NSURL URLWithString:ns_uri] : nil;
            NSData* const data = url != nil ? [NSData dataWithContentsOfURL:url] : nil;
            if (data != nil && data.length > 0)
            {
                bytes.resize(static_cast<std::size_t>(data.length));
                std::memcpy(bytes.data(), data.bytes, static_cast<std::size_t>(data.length));
            }
        }
        on_result(decode_image_bytes(bytes, "uri", uri));
    }

    void stream_image_source_service::load(i_image_source& source, const cancellation_token& token,
                                           completion on_result)
    {
        auto* stream_src = dynamic_cast<i_stream_image_source*>(&source);
        if (stream_src == nullptr || stream_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const image_bytes bytes = stream_src->get_bytes(token);
        on_result(decode_image_bytes(bytes, "stream", "<bytes:" + std::to_string(bytes.size()) + ">"));
    }

    void font_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* font_src = dynamic_cast<const i_font_image_source*>(&source);
        if (font_src == nullptr || font_src->is_empty())
        {
            on_result(image_source_result{}); // not a font source / empty glyph → nothing rendered
            return;
        }
        NSFont* const ns_font = maui::platform::apple::to_ns_font(font_src->font());
        NSColor* const ns_color = maui::platform::apple::to_ns_color(font_src->color());
        NSImage* const image = image_from_glyph(font_src->glyph(), ns_font, ns_color);
        // Font results are RESOLUTION-DEPENDENT (the rasterized glyph depends on display density).
        on_result(make_result(image, "font", std::string(font_src->glyph()), /*resolution_dependent*/ true));
    }
} // namespace maui::core
