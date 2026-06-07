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

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/file_image_source_service.hpp"
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
    // CFReleases it when the result is dropped. A nil image yields a !loaded() result.
    maui::core::image_source_result make_result(NSImage* image, std::string kind, std::string detail)
    {
        if (image == nil)
        {
            return {};
        }
        void* const retained = (__bridge_retained void*)image; // result owns one reference
        return maui::core::image_source_result{retained, [retained] { CFRelease(retained); }, std::move(kind),
                                               std::move(detail)};
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
} // namespace maui::core
