#pragma once
// maui::core::image_source_result  <=  Microsoft.Maui.IImageSourceServiceResult<T> / ImageSourceServiceResult
//
// The outcome of loading one image source: the produced native image handle plus a disposer that
// releases it. Ported from src/Core/src/ImageSources/IImageSourceServiceResult.cs (Value + IDisposable)
// and ImageSourceServiceResult.cs (the Value + Dispose pair). RAII (PROFILE §8): the disposer fires in
// the destructor — the loader holds the *current* result and drops the previous one on the next load,
// exactly like ImageSourceServiceResultManager (which Dispose()s the old result in BeginLoad()).
//
// `image` is the native handle (an NSImage*, retained, on the apple backend; nullptr on headless — there
// is no native image tree). `loaded` is true when a real image was produced (a non-empty source that
// resolved). The headless mirror fields (`kind` + `detail`) let the headless tests observe *what* loaded
// without a native handle: kind is a short tag ("file"/"uri"/"stream"), detail is the resolved path/uri
// (or "<bytes:N>" for a stream). On apple they are left empty.
//
// Move-only: it owns the native handle + the disposer. Copying would double-free.

#include <cstddef>
#include <string>
#include <utility>

#include "maui/core/move_only_function.hpp"

namespace maui::core
{
    class image_source_result
    {
    public:
        // The disposer that releases `image` (e.g. CFRelease on apple). Empty when there is nothing to free.
        using disposer = move_only_function<void()>;

        image_source_result() = default;

        // A loaded native image + its disposer (apple), or just the mirror fields (headless: image == null).
        // resolution_dependent mirrors C# IImageSourceServiceResult.IsResolutionDependent — true for a result
        // whose pixels depend on the display density (the font service sets it; file/uri/stream leave false).
        image_source_result(void* image, disposer dispose, std::string kind, std::string detail,
                            bool resolution_dependent = false)
            : image_(image), dispose_(std::move(dispose)), kind_(std::move(kind)), detail_(std::move(detail)),
              loaded_(true), resolution_dependent_(resolution_dependent)
        {
        }

        image_source_result(const image_source_result&) = delete;
        image_source_result& operator=(const image_source_result&) = delete;
        image_source_result(image_source_result&& other) noexcept
            : image_(other.image_), dispose_(std::move(other.dispose_)), kind_(std::move(other.kind_)),
              detail_(std::move(other.detail_)), loaded_(other.loaded_),
              resolution_dependent_(other.resolution_dependent_)
        {
            other.image_ = nullptr;
            other.loaded_ = false;
            other.resolution_dependent_ = false;
        }
        image_source_result& operator=(image_source_result&& other) noexcept
        {
            if (this != &other)
            {
                dispose();
                image_ = other.image_;
                dispose_ = std::move(other.dispose_);
                kind_ = std::move(other.kind_);
                detail_ = std::move(other.detail_);
                loaded_ = other.loaded_;
                resolution_dependent_ = other.resolution_dependent_;
                other.image_ = nullptr;
                other.loaded_ = false;
                other.resolution_dependent_ = false;
            }
            return *this;
        }
        ~image_source_result()
        {
            dispose();
        }

        [[nodiscard]] void* image() const noexcept
        {
            return image_;
        }
        [[nodiscard]] bool loaded() const noexcept
        {
            return loaded_;
        }
        [[nodiscard]] const std::string& kind() const noexcept
        {
            return kind_;
        }
        [[nodiscard]] const std::string& detail() const noexcept
        {
            return detail_;
        }
        // True when the loaded image's pixels depend on the display density (C# IsResolutionDependent).
        // The loader records this on complete so a later density change can trigger a reload (RequiresReload).
        [[nodiscard]] bool is_resolution_dependent() const noexcept
        {
            return resolution_dependent_;
        }

    private:
        // Release the native handle once (idempotent — clears the disposer after firing).
        void dispose() noexcept
        {
            if (dispose_)
            {
                dispose_();
                dispose_ = nullptr;
            }
            image_ = nullptr;
            loaded_ = false;
            resolution_dependent_ = false;
        }

        void* image_ = nullptr;
        disposer dispose_;
        std::string kind_;
        std::string detail_;
        bool loaded_ = false;
        bool resolution_dependent_ = false;
    };
} // namespace maui::core
