// image_source_service_registry — non-template members: probe-walk resolution + the process-wide default
// registry. The register_service<> template + probes live in the header. See image_source_service_registry.hpp.

#include "maui/core/image_source_service_registry.hpp"

#include <algorithm>
#include <memory>

#include "maui/core/i_image_source.hpp"
#include "maui/core/i_image_source_service.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    std::shared_ptr<i_image_source_service> image_source_service_registry::resolve(const i_image_source& source) const
    {
        for (const auto& e : entries_)
        {
            if (e.probe(source))
            {
                if (!e.cached)
                {
                    e.cached = e.factory(); // first use: create + cache (services are stateless this cut)
                }
                return e.cached;
            }
        }
        return nullptr;
    }

    bool image_source_service_registry::is_registered(type_tag source_type) const
    {
        return std::ranges::any_of(entries_, [source_type](const entry& e) { return e.key == source_type; });
    }

    image_source_service_registry& default_image_source_service_registry()
    {
        static image_source_service_registry registry;
        return registry;
    }
} // namespace maui::core
