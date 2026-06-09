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
// CACHE: an in-memory map keyed by uri holds the fetched bytes plus the (virtual) time they were cached
// (uri_cache_), so a repeat load of the same cached uri skips the re-fetch — UNLESS the entry is older than
// the source's CacheValidity (i_uri_image_source::cache_validity()), in which case it is a MISS and the
// bytes are re-fetched. The "current time" comes from an INJECTED CLOCK SEAM (set_clock) so headless tests
// advance it deterministically; it defaults to std::chrono::steady_clock — testable logic never reads
// wall-clock directly. DEFERRED: a true on-disk cache (C# writes to FileSystem.CacheDirectory). Resolution-
// dependent reload (RequiresReload) is handled by the handler via a separate scale seam. See uri_bytes.hpp
// / i_uri_image_source.hpp.

#include <chrono>
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

        // The injected clock: returns "now" as a steady time point. Defaults to steady_clock::now; tests
        // swap in a controllable clock so CacheValidity expiry is deterministic (no wall-clock dependency).
        using clock = move_only_function<std::chrono::steady_clock::time_point()>;

        // Pushed the in-flight loading state (C# IImageSourcePart.UpdateIsLoading): true when a load begins,
        // false on complete (gated by the source-identity recheck, like the apply). Optional.
        using loading_callback = move_only_function<void(bool)>;

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
        // Inject the clock used to timestamp + expire uri cache entries (CacheValidity). Tests advance a
        // controlled time point here; production leaves the steady_clock default.
        void set_clock(clock now)
        {
            now_ = std::move(now);
        }
        // The current display density, captured into CurrentResolution at complete_load (C#'s
        // uiContext.GetDisplayDensity()). The handler sets this from its screen-DPI seam; defaults to 1.0.
        void set_scale(float scale)
        {
            scale_ = scale;
        }

        // ImageSourceServiceResultManager.BeginLoad: dispose the previous result, cancel the previous
        // token, return a fresh token for the new load.
        cancellation_token begin_load();
        // ImageSourceServiceResultManager.CompleteLoad: take ownership of the current result + clear the token.
        void complete_load(image_source_result result);

        // ImageSourcePartLoader.UpdateImageSourceAsync: (re)load `source` into the view via `apply`.
        // A null/empty source clears (apply is invoked with a `!loaded()` result, then complete_load).
        // Otherwise resolves the service and loads; the apply + complete run only if still current.
        // `on_loading` (optional) receives the in-flight loading state (true at begin, false on complete,
        // matching ImageSourcePartExtensions.UpdateIsLoading + the finally clause).
        void update_source(i_image_source* source, apply_callback apply, loading_callback on_loading = {});

        // Test/inspection: whether a load is in flight (a live, non-cancelled token) — C# IsLoading.
        [[nodiscard]] bool is_loading() const
        {
            return !token_.is_cancelled() && loading_;
        }

        // ImageSourceServiceResultManager.RequiresReload: true when the last-loaded result was
        // resolution-dependent AND the display density differs from the one captured at load time. The
        // handler calls this on a window/DPI change (its OnWindowChanged) and re-issues the source if true.
        // `scale` is the current display density (the handler's screen DPI seam; 1.0 = the load-time default
        // until a real density is provided). DEVIATION: reading the native screen density is the handler's
        // documented seam — this method only compares the captured value against the supplied scale.
        [[nodiscard]] bool requires_reload(float scale) const
        {
            return resolution_dependent_ && current_resolution_ != scale;
        }

        // Test/inspection: the display density captured at the last completed load (C# CurrentResolution).
        [[nodiscard]] float current_resolution() const
        {
            return current_resolution_;
        }
        // Test/inspection: whether the last completed load was resolution-dependent (C# IsResolutionDependent).
        [[nodiscard]] bool is_resolution_dependent() const
        {
            return resolution_dependent_;
        }

    private:
        // Resolve the registry (lazily populating the default one with the built-in services).
        [[nodiscard]] image_source_service_registry& registry();
        // Marshal the apply+complete for a delivered result onto the dispatcher (or run it inline when no
        // dispatcher is set). Runs only if the load is still current (token live + source unchanged); the
        // gated completion also fires on_loading(false) (the C# finally clause).
        void deliver(i_image_source* source, cancellation_token token, image_source_result result, apply_callback apply,
                     loading_callback on_loading = {});
        // Current time from the injected clock (steady_clock by default).
        [[nodiscard]] std::chrono::steady_clock::time_point now() const;

        // A cached uri's fetched bytes plus the (virtual) time they were stored — for CacheValidity expiry.
        struct cache_entry
        {
            image_bytes bytes;
            std::chrono::steady_clock::time_point cached_at;
        };

        i_dispatcher* dispatcher_ = nullptr;
        image_source_service_registry* registry_ = nullptr;
        clock now_;                                    // injected clock; empty => steady_clock::now (see now())
        cancellation_token token_;                     // the current load's token (cancelled when superseded)
        image_source_result current_result_;           // the applied result (disposed on the next begin_load)
        i_image_source* current_source_ = nullptr;     // identity-recheck target (the last source requested)
        std::map<std::string, cache_entry> uri_cache_; // in-memory uri → fetched bytes + cache timestamp
        float scale_ = 1.0F;                           // current display density (set by the handler)
        float current_resolution_ = 1.0F;              // density captured at the last load (C# CurrentResolution)
        bool resolution_dependent_ = false;            // last result was resolution-dependent (C# flag)
        bool loading_ = false;
        // A liveness token: a queued apply captures a weak_ptr to it and bails if the loader was destroyed
        // before the dispatcher ran it (the manual_dispatcher_timer pattern) — UAF-safe teardown.
        std::shared_ptr<int> life_ = std::make_shared<int>(0);
    };
} // namespace maui::core
