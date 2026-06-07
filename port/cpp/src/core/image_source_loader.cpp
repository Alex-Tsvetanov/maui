// image_source_loader — the per-handler async image-load coordinator. See image_source_loader.hpp.
// Ports ImageSourcePartLoader.UpdateImageSourceAsync + ImageSourceServiceResultManager (begin/complete +
// the cancellation lifecycle + prior-result dispose + the source-identity recheck before applying).

#include "maui/core/image_source_loader.hpp"

#include <atomic>
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
        // Take ownership of the now-current result + clear the token (ResultManager.CompleteLoad).
        current_result_ = std::move(result);
        loading_ = false;
    }

    void image_source_loader::deliver(i_image_source* source, cancellation_token token, image_source_result result,
                                      apply_callback apply)
    {
        // The closure that runs the source-identity recheck, then applies + completes. Captured by the
        // dispatcher (or run inline). A weak liveness token guards against the loader being destroyed
        // before a queued apply runs (UAF-safe teardown).
        std::weak_ptr<int> const life = life_;
        auto run = [this, life, source, token = std::move(token), result = std::move(result),
                    apply = std::move(apply)]() mutable {
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

    void image_source_loader::update_source(i_image_source* source, apply_callback apply)
    {
        // Record the source for the identity recheck, then begin a fresh load (cancels the previous one).
        current_source_ = source;
        const cancellation_token token = begin_load();

        if (source == nullptr || source->is_empty())
        {
            // Nothing to load → clear (apply with a !loaded() result), then complete.
            deliver(source, token, image_source_result{}, std::move(apply));
            return;
        }

        // URI sources take the loader's cached fast-path: fetch (or reuse cached) bytes, then decode with
        // the shared per-backend primitive. The in-memory cache keys on the uri (no expiry this cut). The
        // uri_image_source_service stays registered for resolution/DI; its standalone load is the
        // non-cached equivalent of this path.
        if (auto* uri_src = dynamic_cast<i_uri_image_source*>(source))
        {
            const std::string uri(uri_src->uri());
            const bool caching = uri_src->caching_enabled();

            image_bytes bytes;
            const auto cached = caching ? uri_cache_.find(uri) : uri_cache_.end();
            if (cached != uri_cache_.end())
            {
                bytes = cached->second;
            }
            else
            {
                bytes = read_uri_bytes(uri);
                if (caching && !bytes.empty())
                {
                    uri_cache_.emplace(uri, bytes);
                }
            }

            deliver(source, token, decode_image_bytes(bytes, "uri", uri), std::move(apply));
            return;
        }

        // File + stream sources resolve their registered service and load through it. The service invokes
        // on_result (synchronously this cut) with the produced result; deliver() marshals the apply.
        const std::shared_ptr<i_image_source_service> service = registry().resolve(*source);
        if (!service)
        {
            // No service registered for this kind → clear rather than guess (matches map_source's behavior).
            deliver(source, token, image_source_result{}, std::move(apply));
            return;
        }

        i_image_source* const captured_source = source;
        service->load(*source, token,
                      [this, captured_source, token, apply = std::move(apply)](image_source_result result) mutable {
                          deliver(captured_source, token, std::move(result), std::move(apply));
                      });
    }
} // namespace maui::core
