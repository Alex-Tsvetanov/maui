#pragma once
// maui::controls::stream_image_source  <=  Microsoft.Maui.Controls.StreamImageSource
//
// A concrete image source backed by an async bytes provider. Ported from
// src/Controls/src/Core/StreamImageSource.cs, whose `Stream` property is a
// `Func<CancellationToken, Task<Stream>>` and IsEmpty => Stream == null. The port models the provider as
// a move_only_function<image_bytes(const cancellation_token&)> (see i_stream_image_source.hpp for why a
// bytes buffer replaces Task<Stream>); is_empty() mirrors C#'s `Stream == null` as "no provider set".
//
// The provider is move-only (it may capture move-only state, e.g. an owned buffer), so this source is
// itself move-only / non-copyable — consistent with PROFILE §8 (heap-only, shared_ptr-owned elements;
// the image control holds the source as a shared_ptr). get_bytes() invokes the provider on whatever
// thread the loader's service calls it from, honoring the token.
//
// Minted via image_source::from_stream(provider) (the factory lives alongside from_file).

#include <utility>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::controls
{
    class stream_image_source : public maui::core::i_stream_image_source
    {
    public:
        // The bytes producer: given a cancellation token, return the encoded image bytes (empty on
        // cancel / no data). Mirrors C#'s Func<CancellationToken, Task<Stream>>, collapsed to bytes.
        using bytes_provider =
            maui::core::move_only_function<maui::core::image_bytes(const maui::core::cancellation_token&)>;

        explicit stream_image_source(bytes_provider provider) : provider_(std::move(provider))
        {
        }

        // C# StreamImageSource.IsEmpty => Stream == null, modeled as "no provider set".
        [[nodiscard]] bool is_empty() const override
        {
            return !static_cast<bool>(provider_);
        }

        [[nodiscard]] maui::core::image_bytes get_bytes(const maui::core::cancellation_token& token) override
        {
            if (!static_cast<bool>(provider_) || token.is_cancelled())
            {
                return {};
            }
            return provider_(token);
        }

    private:
        bytes_provider provider_;
    };
} // namespace maui::controls
