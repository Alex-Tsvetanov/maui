// image_source_loader — the per-handler async image-load coordinator. See image_source_loader.hpp.
// Ports ImageSourcePartLoader.UpdateImageSourceAsync + ImageSourceServiceResultManager (begin/complete +
// the cancellation lifecycle + prior-result dispose + the source-identity recheck before applying).

#include "maui/core/image_source_loader.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/dispatcher.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_image_source_service.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/i_uri_image_source.hpp"
#include "maui/core/image_decode.hpp"
#include "maui/core/image_source_service_registry.hpp"
#include "maui/core/image_source_services.hpp"
#include "maui/core/uri_bytes.hpp"

namespace maui::core
{
    std::chrono::steady_clock::time_point image_source_loader::now() const
    {
        // The injected clock if set, else steady_clock — testable logic never reads wall-clock directly.
        return now_ ? now_() : std::chrono::steady_clock::now();
    }

    image_source_service_registry& image_source_loader::registry()
    {
        if (registry_ == nullptr)
        {
            // Lazily populate + adopt the process-wide default registry with the built-in services. This
            // names the concrete services from a guaranteed-linked TU (the loader), so the registration is
            // explicit + tree-shake-safe (PROFILE §6).
            registry_ = &default_image_source_service_registry();
            register_default_image_source_services(*registry_);
        }
        return *registry_;
    }

    cancellation_token image_source_loader::begin_load()
    {
        // Dispose the previous result + cancel the previous token (ImageSourceServiceResultManager.BeginLoad).
        current_result_ = image_source_result{};
        token_.cancel();
        token_ = cancellation_token{std::make_shared<std::atomic<bool>>(false)};
        loading_ = true;
        return token_;
    }

    void image_source_loader::complete_load(image_source_result result)
    {
        // Capture resolution-dependence + the current density BEFORE moving the result (C#
        // ResultManager.CompleteLoad<T>: IsResolutionDependent = result.IsResolutionDependent;
        // CurrentResolution = uiContext.GetDisplayDensity()). A !loaded() result resets both (the
        // disposable overload's IsResolutionDependent=false / CurrentResolution=1.0 path).
        if (result.loaded())
        {
            resolution_dependent_ = result.is_resolution_dependent();
            current_resolution_ = scale_;
        }
        else
        {
            resolution_dependent_ = false;
            current_resolution_ = 1.0F;
        }
        // Take ownership of the now-current result + clear the token (ResultManager.CompleteLoad).
        current_result_ = std::move(result);
        loading_ = false;
    }

    void image_source_loader::deliver(i_image_source* source, cancellation_token token, image_source_result result,
                                      apply_callback apply, loading_callback on_loading)
    {
        // The closure that runs the source-identity recheck, then applies + completes. Captured by the
        // dispatcher (or run inline). A weak liveness token guards against the loader being destroyed
        // before a queued apply runs (UAF-safe teardown).
        std::weak_ptr<int> const life = life_;
        auto run = [this, life, source, token = std::move(token), result = std::move(result), apply = std::move(apply),
                    on_loading = std::move(on_loading)]() mutable {
            if (life.expired())
            {
                return; // the loader was destroyed before this apply ran
            }
            // Source-identity recheck (ImageSourcePartExtensions.UpdateSourceAsync):
            //   applied = !token.IsCancellationRequested && imageSource == image.Source
            const bool applied = !token.is_cancelled() && source == current_source_;
            if (!applied)
            {
                return; // superseded by a newer load — drop (result disposes here)
            }
            if (apply)
            {
                apply(result);
            }
            complete_load(std::move(result));
            // The finally clause: mark loading finished (gated by the same identity recheck above — C#'s
            // `if (imageSource == image.Source) image.UpdateIsLoading(false)`).
            if (on_loading)
            {
                on_loading(false);
            }
        };

        if (dispatcher_ != nullptr)
        {
            dispatcher_->dispatch(std::move(run));
        }
        else
        {
            run(); // no dispatcher: apply inline (apple's fast, synchronous file:// / in-memory decode)
        }
    }

    void image_source_loader::update_source(i_image_source* source, apply_callback apply, loading_callback on_loading)
    {
        // Record the source for the identity recheck, then begin a fresh load (cancels the previous one).
        current_source_ = source;
        const cancellation_token token = begin_load();

        if (source == nullptr || source->is_empty())
        {
            // Nothing to load → not loading (C# UpdateIsLoading(false) at entry), clear, then complete.
            if (on_loading)
            {
                on_loading(false);
            }
            deliver(source, token, image_source_result{}, std::move(apply));
            return;
        }

        // A real load is starting (C# events?.LoadingStarted(); image.UpdateIsLoading(true)).
        if (on_loading)
        {
            on_loading(true);
        }

        // URI sources take the loader's cached fast-path: fetch (or reuse cached) bytes, then decode with
        // the shared per-backend primitive. The in-memory cache keys on the uri and EXPIRES entries older
        // than the source's CacheValidity (read against the injected clock) — an expired entry is a miss and
        // is re-fetched + re-stamped (mirrors UriImageSource's CacheValidity governing the cached image's
        // lifetime). The uri_image_source_service stays registered for resolution/DI; its standalone load is
        // the non-cached equivalent of this path.
        if (auto* uri_src = dynamic_cast<i_uri_image_source*>(source))
        {
            const std::string uri(uri_src->uri());
            const bool caching = uri_src->caching_enabled();
            const std::chrono::milliseconds validity = uri_src->cache_validity();
            const std::chrono::steady_clock::time_point at = now();

            image_bytes bytes;
            const auto cached = caching ? uri_cache_.find(uri) : uri_cache_.end();
            const bool fresh = cached != uri_cache_.end() && (at - cached->second.cached_at) < validity;
            if (fresh)
            {
                bytes = cached->second.bytes; // a still-valid cached entry: reuse without re-fetching
            }
            else
            {
                bytes = read_uri_bytes(uri);
                if (caching && !bytes.empty())
                {
                    // Insert or refresh (an expired entry is overwritten with the new bytes + timestamp).
                    uri_cache_[uri] = cache_entry{.bytes = bytes, .cached_at = at};
                }
            }

            deliver(source, token, decode_image_bytes(bytes, "uri", uri), std::move(apply), std::move(on_loading));
            return;
        }

        // File + stream + font sources resolve their registered service and load through it. The service
        // invokes on_result (synchronously this cut) with the produced result; deliver() marshals the apply.
        const std::shared_ptr<i_image_source_service> service = registry().resolve(*source);
        if (!service)
        {
            // No service registered for this kind → clear rather than guess (matches map_source's behavior).
            deliver(source, token, image_source_result{}, std::move(apply), std::move(on_loading));
            return;
        }

        i_image_source* const captured_source = source;
        service->load(*source, token,
                      [this, captured_source, token, apply = std::move(apply),
                       on_loading = std::move(on_loading)](image_source_result result) mutable {
                          deliver(captured_source, token, std::move(result), std::move(apply), std::move(on_loading));
                      });
    }
} // namespace maui::core
