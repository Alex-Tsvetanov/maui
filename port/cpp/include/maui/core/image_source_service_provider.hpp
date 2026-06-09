#pragma once
// maui::core::image_source_service_provider  <=  Microsoft.Maui.IImageSourceServiceProvider +
//                                                Microsoft.Maui.Hosting.ImageSourceServiceProvider
//
// The typed image-source service provider — the port of C#'s IImageSourceServiceProvider /
// ImageSourceServiceProvider, LAYERED over the existing flat image_source_service_registry (it does NOT
// replace it). In MAUI the provider wraps an IServiceProvider + an ImageSourceToImageSourceServiceTypeMapping
// and resolves a service from a source's System.Type by reflection. C++23 has no reflection, so the
// underlying registry resolves from a source INSTANCE via dynamic_cast probes (PROFILE §6); this provider is
// the thin typed face MAUI's image stack resolves through:
//
//   provider.get_image_source_service(source)            -> shared_ptr<i_image_source_service> (or nullptr)
//   provider.get_required_image_source_service(source)   -> throws std::runtime_error if none is registered
//
// It also exposes register_service<Source, Service>() (forwarding to the wrapped registry) so a provider can
// be configured standalone, and a host_service_provider() seam mirroring C#'s HostServiceProvider (here a
// pointer to the registry it draws from). The registry-resolution rules (first registered interface match
// wins; ambiguity is registration-order, not thrown — documented on image_source_service_registry) are
// inherited unchanged; the provider only adds the typed "required" throw + the get-by-instance surface.
//
// DEVIATIONS vs C#:
//   * Resolution is by source instance (dynamic_cast), not System.Type — the no-reflection consequence
//     (PROFILE §6). The obsolete GetImageSourceType / GetImageSourceServiceType (Type→Type) overloads are
//     omitted (they exist in C# only for the legacy reflection path).
//   * The "most-derived interface" + "concrete-over-interface" precedence of
//     ImageSourceToImageSourceServiceTypeMapping is reduced to the registry's registration-order probe
//     walk (our concrete sources each implement exactly one source interface). Documented on the registry.

#include <memory>
#include <stdexcept>
#include <utility>

#include "maui/core/image_source_service_registry.hpp"

namespace maui::core
{
    class i_image_source;
    class i_image_source_service;

    class image_source_service_provider
    {
    public:
        // Build a provider over its own (empty) registry — register services via register_service<>.
        image_source_service_provider()
            : owned_(std::make_unique<image_source_service_registry>()), registry_(owned_.get())
        {
        }

        // Build a provider over an EXISTING registry (non-owning; the registry must outlive the provider).
        // Use this to layer the typed provider over the process-wide default_image_source_service_registry()
        // (or a host-configured one) without re-registering services.
        explicit image_source_service_provider(image_source_service_registry& registry) : registry_(&registry)
        {
        }

        // Register the service that loads sources implementing Source (forwards to the wrapped registry).
        template <class Source, class Service> void register_service()
        {
            registry_->register_service<Source, Service>();
        }

        // C# IImageSourceServiceProvider.GetImageSourceService: the service for `source`, or nullptr if none
        // is registered for any interface it implements.
        [[nodiscard]] std::shared_ptr<i_image_source_service> get_image_source_service(
            const i_image_source& source) const
        {
            return registry_->resolve(source);
        }

        // C# ImageSourceServiceProviderExtensions.GetRequiredImageSourceService: like the above but throws a
        // std::runtime_error (the C++ stand-in for the InvalidOperationException C# raises) when no service
        // is registered for the source.
        [[nodiscard]] std::shared_ptr<i_image_source_service> get_required_image_source_service(
            const i_image_source& source) const
        {
            auto service = registry_->resolve(source);
            if (!service)
            {
                throw std::runtime_error(
                    "Unable to find an image source service for the image source. Register one via "
                    "image_source_service_provider::register_service<Source, Service>().");
            }
            return service;
        }

        // The registry this provider resolves through (C# HostServiceProvider analog — the backing store).
        [[nodiscard]] image_source_service_registry& registry() const
        {
            return *registry_;
        }

    private:
        std::unique_ptr<image_source_service_registry> owned_; // non-null only for the default-ctor case
        image_source_service_registry* registry_;              // the active registry (owned_ or external)
    };
} // namespace maui::core
