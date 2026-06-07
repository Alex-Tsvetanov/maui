#pragma once
// maui::core::i_stream_image_source  <=  Microsoft.Maui.IStreamImageSource
//
// The virtual-view contract for an image sourced from an async byte stream. Ported from
// src/Core/src/ImageSources/IStreamImageSource.cs:
//     Task<Stream> GetStreamAsync(CancellationToken cancellationToken = default);
//
// SIMPLIFICATION vs C#: C# returns a Task<Stream>; the port instead models the source as a *bytes
// provider* — a function the loader invokes ON A WORKER to produce the raw encoded image bytes
// (image_bytes = std::vector<std::byte>). This collapses the `Task<Stream>` (an async producer of a
// readable stream) into "produce the full byte buffer", which is sufficient for the loader (it decodes
// the whole buffer to a native image) and avoids porting the full async-Stream machinery + coroutines
// this cut. The provider takes the cancellation_token so a long-running fetch can bail; an empty
// (zero-length) result means "nothing to load" / cancelled, mirroring C#'s null-stream return.
//
// Threading: get_bytes() may be called off the UI thread (the loader's service runs it before marshalling
// the apply back through the dispatcher). The concrete stream_image_source's provider therefore must be
// safe to invoke on a worker; the in-memory-bytes provider used by tests trivially is.

#include <cstddef>
#include <vector>

#include "maui/core/i_image_source.hpp"

namespace maui::core
{
    class cancellation_token;

    // The raw, still-encoded image bytes (PNG/JPEG/… file contents) a stream source yields.
    using image_bytes = std::vector<std::byte>;

    // Inherits the virtual destructor + protected copy/move from i_image_source (the layered-interface
    // convention). Non-const get_bytes: producing the bytes may advance/consume the underlying provider.
    class i_stream_image_source : public i_image_source
    {
    public:
        // Produce the encoded image bytes (C# IStreamImageSource.GetStreamAsync, simplified to a byte
        // buffer). Returns empty when the source is empty or the token is already cancelled. May run on a
        // worker thread; honors `token` cooperatively.
        [[nodiscard]] virtual image_bytes get_bytes(const cancellation_token& token) = 0;
    };
} // namespace maui::core
