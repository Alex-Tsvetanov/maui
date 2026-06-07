#pragma once
// maui::core::register_default_image_source_services — populate a registry with the built-in services.
//
// The cross-platform registration glue (the C++ stand-in for AppHostBuilderExtensions wiring the default
// IImageSourceServices in MAUI hosting): it maps each built-in source interface to its service —
//   i_file_image_source   -> file_image_source_service
//   i_uri_image_source    -> uri_image_source_service
//   i_stream_image_source -> stream_image_source_service
// This naming is backend-agnostic (each service's load() is the per-backend partial), so the function
// lives in maui_core and is guaranteed-linked via the loader (which calls it to lazily populate the
// default registry on first construction — PROFILE §6's explicit, tree-shake-safe registration).

namespace maui::core
{
    class image_source_service_registry;

    // Register the three built-in source→service mappings into `registry` (idempotent: re-registering a
    // source interface replaces its prior entry). Called by the loader on the default registry.
    void register_default_image_source_services(image_source_service_registry& registry);
} // namespace maui::core
