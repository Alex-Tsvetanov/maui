#pragma once
// maui::core::image_source_service_registry  <=  Microsoft.Maui.Hosting.ImageSourceServiceProvider +
// ImageSourceToImageSourceServiceTypeMapping
//
// The explicit, RTTI-free source-kind → service table (PROFILE §6), mirroring handler_registry. MAUI maps
// a source's System.Type to its IImageSourceService by reflection (interface-assignability matching, with
// the resolved service cached); C++23 has no reflection, so registration is EXPLICIT:
//   register_service<i_file_image_source, file_image_source_service>()
// records a service keyed by the source *interface* it handles, and resolve(source) returns the first
// registered service whose interface the source implements (probed via dynamic_cast — the same mechanism
// image_handler already uses to recognize source kinds).
//
// SIMPLIFICATIONS vs C#'s ImageSourceToImageSourceServiceTypeMapping:
//   * Keyed on the source *interface* only (concrete-type vs interface split collapsed) — our concrete
//     sources each implement exactly one source interface, so a single probe walk suffices.
//   * No ambiguity detection: C# throws when a source matches two unrelated registered interfaces. Here
//     the first registered match wins (registration order). Documented, not enforced.
//   * Service instances are cached per registry (like C#'s _serviceCache) — the services are stateless
//     this cut, so one shared instance is reused across loads.

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/core/i_image_source.hpp"
#include "maui/core/i_image_source_service.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    class image_source_service_registry
    {
    public:
        // Register the service that loads sources implementing Source (an i_*_image_source interface).
        // Re-registering the same Source interface replaces the prior entry.
        template <class Source, class Service> void register_service()
        {
            static_assert(std::is_base_of_v<i_image_source, Source>, "Source must derive maui::core::i_image_source");
            static_assert(std::is_base_of_v<i_image_source_service, Service>,
                          "Service must derive maui::core::i_image_source_service");
            const type_tag key = type_tag::of<Source>();
            entry e{key, &probe_for<Source>,
                    [] { return std::static_pointer_cast<i_image_source_service>(std::make_shared<Service>()); },
                    nullptr};
            for (auto& existing : entries_)
            {
                if (existing.key == key)
                {
                    existing = std::move(e);
                    return;
                }
            }
            entries_.push_back(std::move(e));
        }

        // Find the service for `source`, or nullptr if none is registered for any interface it implements.
        // The instance is created on first use and cached (registration order breaks any ambiguity).
        [[nodiscard]] std::shared_ptr<i_image_source_service> resolve(const i_image_source& source) const;

        [[nodiscard]] bool is_registered(type_tag source_type) const;

    private:
        using probe_fn = bool (*)(const i_image_source&);
        using factory_fn = std::shared_ptr<i_image_source_service> (*)();

        template <class Source> static bool probe_for(const i_image_source& s)
        {
            return dynamic_cast<const Source*>(&s) != nullptr;
        }

        struct entry
        {
            type_tag key;
            probe_fn probe;
            factory_fn factory;
            mutable std::shared_ptr<i_image_source_service> cached; // lazily created, reused across loads
        };

        std::vector<entry> entries_;
    };

    // A process-wide default registry the per-source services self-register into (mirrors
    // default_handler_registry()). The image_source_loader resolves from it when none is injected.
    [[nodiscard]] image_source_service_registry& default_image_source_service_registry();
} // namespace maui::core
