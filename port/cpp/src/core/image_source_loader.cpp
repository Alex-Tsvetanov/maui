// image_source_loader — the per-handler async image-load coordinator. See image_source_loader.hpp.
// Ports ImageSourcePartLoader.UpdateImageSourceAsync + ImageSourceServiceResultManager (begin/complete +
// the cancellation lifecycle + prior-result dispose + the source-identity recheck before applying).

#include "maui/core/image_source_loader.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
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
#include "maui/core/uri_image_disk_cache.hpp"

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
                                      apply_callback apply, loading_callback on_loading, dispatch_prologue prologue)
    {
        // The closure that runs the (ungated) prologue, then the source-identity recheck, then applies +
        // completes. Captured by the dispatcher (or run inline). A weak liveness token guards against the
        // loader being destroyed before a queued apply runs (UAF-safe teardown).
        std::weak_ptr<int> const life = life_;
        auto run = [this, life, source, token = std::move(token), result = std::move(result), apply = std::move(apply),
                    on_loading = std::move(on_loading), prologue = std::move(prologue)]() mutable {
            if (life.expired())
            {
                return; // the loader was destroyed before this apply ran
            }
            // The cache-write prologue runs UNGATED on the dispatcher thread (C# caches even a superseded
            // download), and before the recheck so a later same-uri load sees the populated cache.
            if (prologue)
            {
                prologue();
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

        // URI sources take the loader's cached fast-path (two cache layers + the possibly-async fetch). The
        // uri_image_source_service stays registered for resolution/DI; its standalone load is the non-cached
        // equivalent of this path.
        if (auto* uri_src = dynamic_cast<i_uri_image_source*>(source))
        {
            update_uri_source(source, token, std::string(uri_src->uri()), uri_src->caching_enabled(),
                              uri_src->cache_validity(), std::move(apply), std::move(on_loading));
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

    void image_source_loader::update_uri_source(i_image_source* source, const cancellation_token& token,
                                                const std::string& uri, bool caching,
                                                std::chrono::milliseconds validity, apply_callback apply,
                                                loading_callback on_loading)
    {
        const std::chrono::steady_clock::time_point at = now();

        // Layer 1 — in-memory cache: a still-valid entry is reused without re-reading disk or the network
        // (UriImageSource's CacheValidity governing the cached image's lifetime, applied via the clock seam).
        if (caching)
        {
            const auto cached = uri_cache_.find(uri);
            if (cached != uri_cache_.end() && (at - cached->second.cached_at) < validity)
            {
                deliver(source, token, decode_image_bytes(cached->second.bytes, "uri", uri), std::move(apply),
                        std::move(on_loading));
                return;
            }

            // Layer 2 — on-disk cache: a fresh persisted entry short-circuits the fetch (C#'s
            // `if (CachingEnabled && IsImageCached) GetCachedImage`). It also repopulates the in-memory layer,
            // PRESERVING the disk entry's original timestamp so the in-memory copy expires on the same
            // schedule (measuring age from the original fetch, not this read).
            if (std::optional<uri_image_disk_cache::hit> disk = disk_cache_.read(uri, at, validity))
            {
                image_source_result decoded = decode_image_bytes(disk->bytes, "uri", uri);
                uri_cache_[uri] = cache_entry{.bytes = std::move(disk->bytes), .cached_at = disk->cached_at};
                deliver(source, token, std::move(decoded), std::move(apply), std::move(on_loading));
                return;
            }
        }

        // Miss → fetch. The fetch seam may run on a background queue (apple NSURLSession) or synchronously
        // (the default read_uri_bytes). Its byte sink hops back through deliver_fetched_uri_bytes, which —
        // guarded by the loader's liveness token — runs on the dispatcher thread, so the cache writes there
        // never race the fetch's thread (the only cross-thread bits are the cancellation atomic in the fetch
        // + the dispatcher hand-off in deliver). The uri/token are captured via a shared/refcount copy (cheap,
        // noexcept) so the type-erased sink holds nothing whose copy could throw.
        std::weak_ptr<int> const life = life_;
        auto uri_ref = std::make_shared<const std::string>(uri);
        auto sink = [this, life, source, token, uri_ref, caching, apply = std::move(apply),
                     on_loading = std::move(on_loading)](const image_bytes& bytes) mutable {
            if (life.expired())
            {
                return; // the loader was destroyed before the fetch completed
            }
            deliver_fetched_uri_bytes(source, token, *uri_ref, caching, bytes, std::move(apply), std::move(on_loading));
        };

        if (uri_fetch_)
        {
            uri_fetch_(uri, token, std::move(sink)); // injected async fetch (apple http)
        }
        else
        {
            sink(read_uri_bytes(uri)); // default synchronous fetch (file:// + local paths)
        }
    }

    void image_source_loader::deliver_fetched_uri_bytes(i_image_source* source, const cancellation_token& token,
                                                        const std::string& uri, bool caching, const image_bytes& bytes,
                                                        apply_callback apply, loading_callback on_loading)
    {
        // This may run on the fetch's background thread (apple NSURLSession) — so it touches NO loader member
        // state: the decode (bytes → native image) is pure/thread-safe work, and the cache write is deferred
        // into the dispatcher PROLOGUE below, which runs on the dispatcher thread. `now()` reads only the
        // injected clock (steady_clock in production; tests use the synchronous fetch, so no race).
        const std::chrono::steady_clock::time_point at = now();
        const bool cacheable = caching && !bytes.empty();

        // The cache-write prologue: persist + memo the fetched bytes on the dispatcher thread, UNGATED (C#
        // caches even a download that is later superseded). The bytes + uri are shared (refcount copies are
        // cheap + noexcept) so the type-erased prologue holds nothing whose copy could throw. A no-op when not
        // cacheable; both the disk write + the in-memory insert read the shared buffer.
        dispatch_prologue populate;
        if (cacheable)
        {
            auto shared_bytes = std::make_shared<const image_bytes>(bytes);
            auto uri_ref = std::make_shared<const std::string>(uri);
            populate = [this, uri_ref, at, shared_bytes]() {
                disk_cache_.write(*uri_ref, *shared_bytes, at); // C# CacheImage; no-op if the disk layer is off
                uri_cache_[*uri_ref] = cache_entry{.bytes = *shared_bytes, .cached_at = at};
            };
        }

        deliver(source, token, decode_image_bytes(bytes, "uri", uri), std::move(apply), std::move(on_loading),
                std::move(populate));
    }
} // namespace maui::core
