#pragma once
// maui::core::image_source_loader  <=  Microsoft.Maui.Platform.ImageSourcePartLoader +
// Microsoft.Maui.ImageSourceServiceResultManager (merged)
//
// The per-handler async image-load coordinator. Ported from ImageSourcePartLoader.UpdateImageSourceAsync
// (resolve the service, load, apply only if still the current source, complete) folded together with
// ImageSourceServiceResultManager (begin/complete + the cancellation-token lifecycle + the prior-result
// dispose). One loader is OWNED by the image handler (like C#'s SourceLoader field).
//
// LIFECYCLE (mirrors C#):
//   begin_load()  : dispose the previous result, CANCEL the previous token, mint + return a fresh token.
//   update_source(): set the current source, begin_load(), resolve the service, kick the load; when the
//                    result arrives, marshal the apply onto the dispatcher and — only if the load is still
//                    current (`!token.is_cancelled() && source == current_source_`, the SOURCE-IDENTITY
//                    RECHECK from ImageSourcePartExtensions.UpdateSourceAsync) — invoke apply + complete_load.
//   complete_load(): take ownership of the (now-current) result, clear the token.
//
// THREADING (PROFILE §8): the load's *apply* is always marshalled through the dispatcher, so it runs on
// the dispatcher thread (the headless manual_dispatcher in tests — pump it to observe the apply). A
// service that loads synchronously (file/stream-from-bytes, this cut) still routes its apply through the
// dispatcher. If no dispatcher is set, the apply runs INLINE (synchronously) — used by the apple tests,
// whose `file://` + in-memory-PNG decodes are fast and synchronous (no real background queue → TSan-clean;
// a production HTTP fetch would move the byte fetch to a background queue and marshal the apply back here).
//
// CACHE: an in-memory map keyed by uri holds the fetched bytes (uri_cache_), so a repeat load of the same
// cached uri skips the re-fetch. DEFERRED: disk caching + CacheValidity expiry, and resolution-dependent
// reload (RequiresReload). See uri_bytes.hpp / i_uri_image_source.hpp.

#include <map>
#include <memory>
#include <string>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/i_stream_image_source.hpp" // image_bytes (uri cache value)
#include "maui/core/image_source_result.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::core
{
    class i_dispatcher;
    class i_image_source;
    class image_source_service_registry;

    class image_source_loader
    {
    public:
        // Invoked (on the dispatcher thread) with a successfully-loaded result that is still current. The
        // handler's closure copies the native image / headless mirror out of the result into its view.
        using apply_callback = move_only_function<void(const image_source_result&)>;

        image_source_loader() = default;
        image_source_loader(const image_source_loader&) = delete;
        image_source_loader& operator=(const image_source_loader&) = delete;
        image_source_loader(image_source_loader&&) = delete;
        image_source_loader& operator=(image_source_loader&&) = delete;
        ~image_source_loader() = default;

        // The dispatcher the apply is marshalled onto (headless: a manual_dispatcher the test pumps;
        // apple: optional — unset means run the apply inline). Non-owning; must outlive the loader.
        void set_dispatcher(i_dispatcher& dispatcher)
        {
            dispatcher_ = &dispatcher;
        }
        // The service registry to resolve sources against (defaults to the process-wide registry,
        // lazily populated with the built-in services on first update_source). Non-owning.
        void set_registry(image_source_service_registry& registry)
        {
            registry_ = &registry;
        }

        // ImageSourceServiceResultManager.BeginLoad: dispose the previous result, cancel the previous
        // token, return a fresh token for the new load.
        cancellation_token begin_load();
        // ImageSourceServiceResultManager.CompleteLoad: take ownership of the current result + clear the token.
        void complete_load(image_source_result result);

        // ImageSourcePartLoader.UpdateImageSourceAsync: (re)load `source` into the view via `apply`.
        // A null/empty source clears (apply is invoked with a `!loaded()` result, then complete_load).
        // Otherwise resolves the service and loads; the apply + complete run only if still current.
        void update_source(i_image_source* source, apply_callback apply);

        // Test/inspection: whether a load is in flight (a live, non-cancelled token) — C# IsLoading.
        [[nodiscard]] bool is_loading() const
        {
            return !token_.is_cancelled() && loading_;
        }

    private:
        // Resolve the registry (lazily populating the default one with the built-in services).
        [[nodiscard]] image_source_service_registry& registry();
        // Marshal the apply+complete for a delivered result onto the dispatcher (or run it inline when no
        // dispatcher is set). Runs only if the load is still current (token live + source unchanged).
        void deliver(i_image_source* source, cancellation_token token, image_source_result result,
                     apply_callback apply);

        i_dispatcher* dispatcher_ = nullptr;
        image_source_service_registry* registry_ = nullptr;
        cancellation_token token_;                     // the current load's token (cancelled when superseded)
        image_source_result current_result_;           // the applied result (disposed on the next begin_load)
        i_image_source* current_source_ = nullptr;     // identity-recheck target (the last source requested)
        std::map<std::string, image_bytes> uri_cache_; // in-memory uri → fetched bytes (no expiry this cut)
        bool loading_ = false;
        // A liveness token: a queued apply captures a weak_ptr to it and bails if the loader was destroyed
        // before the dispatcher ran it (the manual_dispatcher_timer pattern) — UAF-safe teardown.
        std::shared_ptr<int> life_ = std::make_shared<int>(0);
    };
} // namespace maui::core
