#pragma once
// maui::core::i_image_source_service  <=  Microsoft.Maui.IImageSourceService
//
// The service that loads one kind of image source into a native image. Ported from
// src/Core/src/ImageSources/IImageSourceService.cs (per-platform GetImageAsync(source, scale, token) ->
// Task<IImageSourceServiceResult<TImage>>) + ImageSourceService.cs (the abstract base). One concrete
// service per source kind (file / uri / stream); they are registered in an image_source_service_registry
// and resolved by the loader from the source instance.
//
// SIMPLIFICATIONS vs C#:
//   * No coroutines/Task: the async result is delivered through a completion callback `on_result` instead
//     of an awaited Task. A service that loads synchronously (file/stream-from-bytes, this cut) invokes
//     on_result before returning; the seam still lets a future service invoke it later from a worker (the
//     loader marshals the apply through the dispatcher either way).
//   * The `scale` parameter (display density) is dropped — resolution-dependent reload is deferred.
//   * The result carries a `void*` native image (NSImage* on apple) rather than a typed TImage; the
//     headless services fill only the mirror fields (see image_source_result.hpp).

#include "maui/core/image_source_result.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::core
{
    class i_image_source;
    class cancellation_token;

    class i_image_source_service
    {
    public:
        virtual ~i_image_source_service() = default;

        // Invoked with the loaded result (or an empty/`!loaded()` result when nothing was produced —
        // an empty/cancelled source, or a failed load). Mirrors awaiting GetImageAsync's Task result.
        using completion = move_only_function<void(image_source_result)>;

        // Load `source` into a native image, delivering the outcome through `on_result`. `token` lets a
        // long-running load bail cooperatively. C# IImageSourceService.GetImageAsync.
        virtual void load(i_image_source& source, const cancellation_token& token, completion on_result) = 0;

    protected:
        i_image_source_service() = default;
        i_image_source_service(const i_image_source_service&) = default;
        i_image_source_service(i_image_source_service&&) = default;
        i_image_source_service& operator=(const i_image_source_service&) = default;
        i_image_source_service& operator=(i_image_source_service&&) = default;
    };
} // namespace maui::core
