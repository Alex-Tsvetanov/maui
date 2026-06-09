// register_default_image_source_services — cross-platform registration glue. See image_source_services.hpp.
// Maps each built-in source interface to its service; the service load() bodies are the per-backend partials.

#include "maui/core/image_source_services.hpp"

#include "maui/core/file_image_source_service.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/i_font_image_source.hpp"
#include "maui/core/i_image_source.hpp" // i_file_image_source
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/i_uri_image_source.hpp"
#include "maui/core/image_source_service_registry.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_image_source_service.hpp"

namespace maui::core
{
    void register_default_image_source_services(image_source_service_registry& registry)
    {
        registry.register_service<i_file_image_source, file_image_source_service>();
        registry.register_service<i_uri_image_source, uri_image_source_service>();
        registry.register_service<i_stream_image_source, stream_image_source_service>();
        registry.register_service<i_font_image_source, font_image_source_service>();
    }
} // namespace maui::core
